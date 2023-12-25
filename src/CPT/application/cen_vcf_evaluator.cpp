#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/format/json.hpp>

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        std::cout << "cen_vcf_evaluator [calling_genotype.tsv] [mapping_cen_vcf.json]" << std::endl;
        return 1;
    }

    auto call_file = std::ifstream(argv[1]);
    auto json_file = std::ifstream(argv[2]);

    auto mappings = std::map<std::string, std::string>();
    auto maps = boost::property_tree::ptree();
    boost::property_tree::read_json(json_file, maps);

    for (auto &mapping : maps)
    {
        const auto &file = mapping.second.get_value<std::string>();
        mappings[mapping.first] = boost::filesystem::exists(file) ? file : "";
    }

    auto psids_idx = std::map<std::string, int>();
    auto names_idx = std::map<std::string, int>();

    auto call_psids = std::vector<std::string>();
    auto call_names = std::vector<std::string>();
    auto call_gts = std::vector<int>();

    auto line = std::string();
    auto splited = std::vector<std::string>();

    std::cerr << "Reading " << argv[1] << std::endl;

    while (std::getline(call_file, line))
    {
        boost::iter_split(splited, line, boost::algorithm::first_finder("\t"));
        if (line.at(0) == 'x' || line.at(0) == 'a')
            continue;

        auto call_gt = std::stoi(splited[2]);
        auto &call_id = splited[4];
        auto &call_name = splited[5];

        psids_idx.emplace(call_id, 0);
        names_idx.emplace(call_name, 0);

        call_gts.push_back(call_gt);
        call_psids.push_back(call_id);
        call_names.push_back(call_name);
    }

    call_file.close();
    json_file.close();

    auto ps_ids = std::vector<std::string>(psids_idx.size());
    auto cen_names = std::vector<std::string>(names_idx.size());

    auto idx = 0;
    for (auto &psid : psids_idx)
    {
        std::cerr << "Uniquing probeset id: " << idx + 1 << "/" << psids_idx.size() << "\r";
        psid.second = idx;
        ps_ids[idx] = psid.first;
        idx++;
    }

    std::cerr << std::endl;

    idx = 0;
    for (auto &name : names_idx)
    {
        std::cerr << "Uniquing cen name: " << idx + 1 << "/" << names_idx.size() << "\r";
        name.second = idx;
        cen_names[idx] = name.first;
        idx++;
    }

    std::cerr << std::endl;

    auto gt_vcf = std::vector<std::vector<int>>(psids_idx.size(), std::vector<int>(names_idx.size(), -1));  // GT: -1=./., 0=0/0, 1=0/1, 2=1/1
    auto gt_call = std::vector<std::vector<int>>(psids_idx.size(), std::vector<int>(names_idx.size(), -1)); // GT: -1=./., 0=0/0, 1=0/1, 2=1/1
    auto gt_acc = std::vector<std::vector<int>>(psids_idx.size(), std::vector<int>(names_idx.size(), -1));  // ACC: -1=no call, 0=fasle, 1=true

    for (idx = 0; idx < call_gts.size(); idx++)
    {
        std::cerr << "Assigning genotype call: " << idx + 1 << "/" << call_gts.size() << "\r";
        auto &psid = call_psids[idx];
        auto &name = call_names[idx];
        auto &gt = call_gts[idx];

        gt_call[psids_idx[psid]][names_idx[name]] = gt;
    }

    std::cerr << std::endl;
    auto exclude_idx = std::set<int>();
    idx = 0;

    for (auto &cen : names_idx)
    {
        idx++;
        std::cerr << "Reading vcf files: " << idx << "/" << names_idx.size() << "\r";
        const auto &cen_name = cen.first;

        if (mappings.find(cen_name) != mappings.end() && mappings[cen_name] != "")
        {
            auto filein = std::ifstream(mappings[cen_name]);

            while (std::getline(filein, line))
            {
                if (line.at(0) == '#')
                    continue;

                boost::iter_split(splited, line, boost::algorithm::first_finder("\t"));

                auto &vcf_id = splited[2];
                auto &vcf_gt = splited[9];

                if (psids_idx.find(vcf_id) == psids_idx.end())
                    continue;

                auto &call_gt = gt_call[psids_idx[vcf_id]][names_idx[cen_name]];
                boost::iter_split(splited, vcf_gt, boost::algorithm::first_finder("/"));

                if (splited[0] == "1")
                {
                    gt_vcf[psids_idx[vcf_id]][names_idx[cen_name]] = 2;

                    switch (call_gt) // 1/1
                    {
                    case 0:
                        gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 0;
                        break;
                    case 1:
                        gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 0;
                        break;
                    case 2:
                        gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 1;
                        break;
                    }
                }
                else if (splited[0] == "0")
                {
                    if (splited[1] == "0")
                    {
                        gt_vcf[psids_idx[vcf_id]][names_idx[cen_name]] = 0;

                        switch (call_gt) // 0/0
                        {
                        case 0:
                            gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 1;
                            break;
                        case 1:
                            gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 0;
                            break;
                        case 2:
                            gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 0;
                            break;
                        }
                    }
                    else
                    {
                        gt_vcf[psids_idx[vcf_id]][names_idx[cen_name]] = 1;

                        switch (call_gt) // 0/1
                        {
                        case 0:
                            gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 0;
                            break;
                        case 1:
                            gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 1;
                            break;
                        case 2:
                            gt_acc[psids_idx[vcf_id]][names_idx[cen_name]] = 0;
                            break;
                        }
                    }
                }
                else
                {
                    // ./.
                }
            }
        }
        else
        {
            exclude_idx.emplace(cen.second);
        }
    }

    std::cerr << std::endl;

    double t_0 = 0.0, f_0 = 0.0;
    double t_1 = 0.0, f_1 = 0.0;
    double t_2 = 0.0, f_2 = 0.0;

    double t_ps_0 = 0.0, f_ps_0 = 0.0;
    double t_ps_1 = 0.0, f_ps_1 = 0.0;
    double t_ps_2 = 0.0, f_ps_2 = 0.0;

    double acc = 0.0, acc0 = 0.0, acc1 = 0.0, acc2 = 0.0;

    std::vector<double> t_cen_0( cen_names.size(), 0.0 ), f_cen_0( cen_names.size(), 0.0 );
    std::vector<double> t_cen_1( cen_names.size(), 0.0 ), f_cen_1( cen_names.size(), 0.0 );
    std::vector<double> t_cen_2( cen_names.size(), 0.0 ), f_cen_2( cen_names.size(), 0.0 );

    boost::iter_split(splited, argv[1], boost::algorithm::first_finder("/"));
    boost::iter_split(splited, splited[ splited.size() -1], boost::algorithm::first_finder(".tsv"));
    auto out_name = splited[0] + "_evaluated.tsv";

    std::ofstream out_file( out_name );
    out_file << "Probeset_ID(TF|Call|Vcf)\tAcc.(A|0|1|2)";

    for (int i = 0; i < cen_names.size(); i++)
    {
        if (exclude_idx.find(i) != exclude_idx.end())
            continue;

        out_file << "\t" << cen_names[i];
    }

    out_file << std::endl;

    for (idx = 0; idx < gt_acc.size(); idx++)
    {
        std::cerr << "Writing " << out_name << ": " << idx + 1 << "/" << gt_acc.size() << "\r";
        auto &psid = ps_ids[idx];

        t_ps_0 = 0.0, f_ps_0 = 0.0;
        t_ps_1 = 0.0, f_ps_1 = 0.0;
        t_ps_2 = 0.0, f_ps_2 = 0.0;

        for (auto i = 0; i < gt_acc[idx].size(); i++)
        {
            if (exclude_idx.find(i) != exclude_idx.end())
                continue;

            switch (gt_acc[idx][i])
            {
            case 0:
                switch (gt_call[idx][i])
                {
                case 0:
                    f_0++;
                    f_ps_0++;
                    f_cen_0[i]++;
                    break;
                case 1:
                    f_1++;
                    f_ps_1++;
                    f_cen_1[i]++;
                    break;
                case 2:
                    f_2++;
                    f_ps_2++;
                    f_cen_2[i]++;
                    break;
                }
                break;
            case 1:
                switch (gt_vcf[idx][i])
                {
                case 0:
                    t_0++;
                    t_ps_0++;
                    t_cen_0[i]++;
                    break;
                case 1:
                    t_1++;
                    t_ps_1++;
                    t_cen_1[i]++;
                    break;
                case 2:
                    t_2++;
                    t_ps_2++;
                    t_cen_2[i]++;
                    break;
                }
                break;
            }
        }

        acc0 = (t_ps_0 + f_ps_0) == 0 ? -1 : (t_ps_0 * 100 / (t_ps_0 + f_ps_0));
        acc1 = (t_ps_1 + f_ps_1) == 0 ? -1 : (t_ps_1 * 100 / (t_ps_1 + f_ps_1));
        acc2 = (t_ps_2 + f_ps_2) == 0 ? -1 : (t_ps_2 * 100 / (t_ps_2 + f_ps_2));

        acc = (t_ps_0 + t_ps_1 + t_ps_2) * 100 / (t_ps_0 + t_ps_1 + t_ps_2 + f_ps_0 + f_ps_1 + f_ps_2);
        out_file << psid << "\t" << acc << "%|" << acc0 << "%|" << acc1 << "%|" << acc2 << "%";

        for (auto i = 0; i < gt_acc[idx].size(); i++)
        {
            out_file << "\t" << gt_acc[idx][i] << "|" << gt_call[idx][i] << "|" << gt_vcf[idx][i];
        }

        out_file << std::endl;
    }
    
    acc0 = (t_0 + f_0) == 0 ? -1 : (t_0 * 100 / (t_0 + f_0));
    acc1 = (t_1 + f_1) == 0 ? -1 : (t_1 * 100 / (t_1 + f_1));
    acc2 = (t_2 + f_2) == 0 ? -1 : (t_2 * 100 / (t_2 + f_2));

    acc = (t_0 + t_1 + t_2) * 100 / (t_0 + t_1 + t_2 + f_0 + f_1 + f_2);
    out_file << acc << "%|" << acc0 << "%|" << acc1 << "%|" << acc2 << "%\t:All|Samples:";

    std::cerr << "\nACC.\tAll: " << acc << "%\t0/0: " << acc0 << "%\t0/1: " << acc1 << "%\t1/1: " << acc2 << "%" << std::endl;

    for( int i = 0; i < cen_names.size(); i++ )
    {
        if (exclude_idx.find(i) != exclude_idx.end())
            continue;

        t_0 = t_cen_0[i];
        t_1 = t_cen_1[i];
        t_2 = t_cen_2[i];

        f_0 = f_cen_0[i];
        f_1 = f_cen_1[i];
        f_2 = f_cen_2[i];

        acc0 = (t_0 + f_0) == 0 ? -1 : (t_0 * 100 / (t_0 + f_0));
        acc1 = (t_1 + f_1) == 0 ? -1 : (t_1 * 100 / (t_1 + f_1));
        acc2 = (t_2 + f_2) == 0 ? -1 : (t_2 * 100 / (t_2 + f_2));

        acc = (t_0 + t_1 + t_2) * 100 / (t_0 + t_1 + t_2 + f_0 + f_1 + f_2);
        out_file << "\t" << acc << "%|" << acc0 << "%|" << acc1 << "%|" << acc2 << "%";
    }
    
    out_file << std::endl;
    out_file.close();

    std::cerr << "Skiped cen files: " << exclude_idx.size() << std::endl;
    for (auto &cen : names_idx)
    {
        if (exclude_idx.find(cen.second) != exclude_idx.end())
            std::cerr << cen.first << std::endl;
    }

    return 0;
}