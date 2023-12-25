#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>

int main(int argc, char **argv)
{
    boost::program_options::variables_map op;
    boost::program_options::options_description options("Options");
    options.add_options()("help,h", "Print help messages")(",p", boost::program_options::value<std::string>()->required(), "Set input ps list file")(",v", boost::program_options::value<std::string>()->required(), "Set input vcf file list")(",f", boost::program_options::value<std::string>()->required(), "Set input gnome.fna file")(",i", boost::program_options::value<std::string>()->required(), "Set input anno.csv file")(",o", boost::program_options::value<std::string>()->required(), "Set output file name");
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), op);
        if (op.count("help"))
        {
            std::cout << "\n"
                      << options << "\n";
            return 0;
        }
        boost::program_options::notify(op);
    }
    catch (boost::program_options::error &error)
    {
        std::cerr << "\nERROR: " << error.what() << "\n";
        std::cerr << options << "\n";
        return 1;
    }

    std::ifstream inpsl(op["-p"].as<std::string>());
    std::ifstream inlst(op["-v"].as<std::string>());
    std::ifstream infna(op["-f"].as<std::string>());
    std::ifstream input(op["-i"].as<std::string>());
    std::string output(op["-o"].as<std::string>());

    std::string temp = "";
    std::set<std::string> ps_list;

    while (std::getline(inpsl, temp))
    {
        if (temp.empty() || temp == "probeset_id")
            continue;

        ps_list.emplace(temp);
    }

    std::vector<std::string> line, chr_pos;
    std::map<std::string, std::string> vcfs;
    //       chr:pos    probeset_id,ref,alt

    while (std::getline(inlst, temp))
    {
        if (temp.empty())
            continue;

        std::ifstream invcf(temp);

        // #CHROM POS ID REF ALT QUAL FILTER INFO FORMAT
        // 0      1   2  3   4   5    6      7    8
        while (std::getline(invcf, temp))
        {
            boost::iter_split(line, temp, boost::algorithm::first_finder("\t"));

            if (ps_list.find(line[2]) == ps_list.end())
                continue;

            auto pos = line[0] + ":" + line[1];
            auto data = line[2] + "," + line[3] + "," + line[4];

            vcfs.emplace(pos, data);
        }
    }

    auto last_chr = std::string{};
    auto genome = std::map<std::string, std::string>{};

    while (std::getline(infna, temp))
    {
        if (temp.at(0) == '>')
        {
            // std::cerr << "Reading: " << temp << std::endl;
            std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
            boost::iter_split(line, temp, boost::algorithm::first_finder(","));

            if (line[0].find("chromosome") == std::string::npos)
            {
                // std::cerr << "Out of chromosome: " << line[0] << std::endl;
                break;
            }

            boost::iter_split(line, line[0], boost::algorithm::first_finder("chromosome "));
            auto &chr = line[1];

            last_chr = chr == "x" ? "chr39" : "chr" + chr;
            genome[last_chr] = "";

            // std::cerr << "Chromsome: " << last_chr << " begin to read" << std::endl;
            continue;
        }

        std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
        genome[last_chr].append(temp);
    }

    std::set<std::string> biall_ps;
    std::map<std::string, std::vector<std::string>> probesets;
    //          pos       probe_id,x,y,chip_seq,probe_seq,start,site,gap

    int num_probes = 0, max_seq_length = 0;
    int num_rows = 496, num_cols = 496;

    while (std::getline(input, temp))
    {
        if (temp.at(0) == ',')
            continue;

        boost::iter_split(line, temp, boost::algorithm::first_finder(","));
        auto &pos = line[11];
        auto &id = line[5];

        if (vcfs.find(pos) == vcfs.end())
            continue;

        auto x = line[2];
        auto y = std::to_string(495 - std::stoi(line[3]));
        auto &chip_seq = line[4];
        auto &probe_seq = line[6];

        auto probe_id = std::to_string(std::stoi(y) * num_cols + std::stoi(x));

        boost::iter_split(chr_pos, pos, boost::algorithm::first_finder(":"));
        auto &chr = chr_pos[0];

        auto &qstart = line[10];
        auto &site = chr_pos[1];
        auto &gap = line[12];

        auto data = probe_id + "," + x + "," + y + "," + chip_seq + "," + probe_seq + "," + qstart + "," + site + "," + gap;
        probesets[pos].emplace_back(data);

        max_seq_length = std::max(max_seq_length, int(chip_seq.length()));
        num_probes++;
    }

    boost::property_tree::ptree json;

    json.add("magic", "113");
    json.add("version", "000");
    json.add("probe_array_type.name", "DogBanff");
    json.add("probe_array_type.version", "0");
    json.add("num_probesets", num_probes);
    json.add("num_probes", num_probes);
    json.add("num_rows", num_rows);
    json.add("num_cols", num_cols);
    json.add("num_channels", 2);
    json.add("max_chn_items", 2);
    json.add("num_features", num_probes);
    json.add("max_seq_length", max_seq_length);
    json.add("genome_assembly", "CanFam3.1");
    json.add("probe_direction", "3-5");

    std::vector<std::string> vdata, pdata;
    boost::property_tree::ptree probeset_list;

    for (auto &probeset : probesets)
    {
        for (auto &probe : probeset.second)
        {
            boost::property_tree::ptree json_ps;
            boost::property_tree::ptree probe_list;

            boost::iter_split(chr_pos, probeset.first, boost::algorithm::first_finder(":"));

            auto chr = "chr" + chr_pos[0];
            auto end = std::stoi(chr_pos[1]);
            std::string ref = "", alt = "";

            json_ps.add("type", "Genotyping");
            json_ps.add("subtype", "LigationBased");
            json_ps.add("chrom", chr);
            json_ps.add("start", end - 1);
            json_ps.add("end", end);
            json_ps.add("strand", "+");
            json_ps.add("num_probes", probeset.second.size());

            auto &vcf = vcfs[probeset.first];
            boost::iter_split(vdata, vcf, boost::algorithm::first_finder(","));
            boost::iter_split(pdata, probe, boost::algorithm::first_finder(","));

            ref = vdata[1];
            alt = vdata[2];

            auto &probe_id = pdata[0];
            auto &x = pdata[1];
            auto &y = pdata[2];
            auto &chip_seq = pdata[3];
            auto &probe_seq = pdata[4];
            auto qstart = std::stoi(pdata[5]);
            auto site = std::stoi(pdata[6]);
            auto gap = std::stoi(pdata[7]);

            auto probeset_id = vdata[0] + "_" + probe_id;
            auto desc = ref + alt;

            auto is_indel = bool(ref.length() > 1 || alt.length() > 1); // Indel Probe
            desc = is_indel ? "" : desc;

            json_ps.add("name", probeset_id);
            json_ps.add("desc", desc);

            auto gaps = std::set<int>{};

            for (auto &probe : probeset.second)
                gaps.emplace(std::stoi(probe.substr(probe.length() - 1, 1)));

            auto is_gap = bool((gaps.size() == 2 && *gaps.rbegin() == 1) || (gaps.size() == 1 && *gaps.begin() == 1) && !is_indel);                   // Gap Probe
            auto is_indel_gap = bool((gaps.size() >= 2 && *std::next(gaps.begin(), 1) == 1) || (gaps.size() == 1 && *gaps.begin() == 1) && is_indel); // Indel Gap Probe

            if (probeset.first == "5:48804751")
                is_gap = true;

            boost::property_tree::ptree json_p;

            json_p.add("probe_name", probe_id);
            json_p.add("shape_name", "1");

            boost::property_tree::ptree r1, r2, r3, r4;
            boost::property_tree::ptree p_region, p_channel, p_sequence;

            r1.put("", x);
            r2.put("", y);
            r3.put("", 1);
            r4.put("", 1);

            p_region.push_back(std::make_pair("", r1));
            p_region.push_back(std::make_pair("", r2));
            p_region.push_back(std::make_pair("", r3));
            p_region.push_back(std::make_pair("", r4));

            auto fseq = genome[chr].substr(qstart - 1, 8);
            auto is_neg = bool(fseq == probe_seq.substr(0, 8));

            auto allele = "A/B";
            auto is_ref_small = bool(ref < alt);

            std::string ref_cov = ref == "A" ? "T" : (ref == "C" ? "G" : (ref == "G" ? "C" : "A"));
            std::string alt_cov = alt == "A" ? "T" : (alt == "C" ? "G" : (alt == "G" ? "C" : "A"));

            std::string base_ref = is_neg ? ref_cov : ref;
            std::string base_alt = is_neg ? alt_cov : alt;

            std::string base = base_ref + "/" + base_alt;
            std::string channel = (base.at(0) == 'A' || base.at(0) == 'T' ? "1/0" : "0/1");

            if (is_gap)
            {
                fseq = genome[chr].substr(qstart - 1 + gap, 8);
                is_neg = bool(fseq == probe_seq.substr(1, 8));

                allele = gap == 1 ? "A" : "B";
                base = chip_seq.substr(chip_seq.length() - 1, 1);

                channel = is_neg ? genome[chr].substr(qstart - 1 + gap - 2, 1) : genome[chr].substr(qstart - 1 - gap + 2, 1);
                channel = channel == "A" || channel == "T" ? "1" : "0";
            }

            if (is_indel)
            {
                if (is_indel_gap && (gap == 0 || gap == 1))
                {
                    fseq = genome[chr].substr(qstart - 1 + gap, 8);
                    is_neg = bool(fseq == probe_seq.substr(1, 8));
                    base = chip_seq.substr(chip_seq.length() - 1, 1);

                    auto base_pos = is_neg ? (qstart - 1 + gap) : (qstart + 1 - gap);
                    auto channel_pos_ref = is_neg ? (base_pos - 1) : (base_pos + 1);

                    auto channel_base = gap == 1
                                            ? genome[chr].substr(channel_pos_ref - 1, 1)
                                            : (channel_pos_ref > site + alt.length() || (channel_pos_ref < site)
                                                   ? genome[chr].substr(channel_pos_ref - 1, 1)
                                                   : alt.substr(channel_pos_ref - site, 1));

                    channel = channel_base == "A" || channel_base == "T" ? "1" : "0";
                    allele = gap == 1 ? "A" : "B";
                }
                else
                {
                    is_neg = bool(fseq == probe_seq.substr(0, 8));
                    auto base_pos = is_neg ? (qstart - 1) : (qstart + 1);
                    auto base_ref = genome[chr].substr(base_pos - 1, 1);

                    auto base_alt = base_pos > site + alt.length() || (base_pos < site)
                                        ? genome[chr].substr(base_pos - 1, 1)
                                        : alt.substr(base_pos - site, 1);

                    ref_cov = base_ref == "A" ? "T" : (base_ref == "C" ? "G" : (base_ref == "G" ? "C" : "A"));
                    alt_cov = base_alt == "A" ? "T" : (base_alt == "C" ? "G" : (base_alt == "G" ? "C" : "A"));

                    base = is_neg ? (ref_cov + "/" + alt_cov) : (base_ref + "/" + base_alt);
                    channel = (base.at(0) == 'A' || base.at(0) == 'T' ? "1/0" : "0/1");
                }
            }

            p_channel.add("allele", allele);
            p_channel.add("channel", channel);
            p_channel.add("base", base);

            p_sequence.add("start", is_neg ? qstart - 1 + chip_seq.length() : qstart - 1 - chip_seq.length());
            p_sequence.add("length", chip_seq.length());
            p_sequence.add("strand", is_neg ? "-" : "+");
            p_sequence.add("content", chip_seq);

            json_p.add_child("region_des", p_region);
            json_p.add_child("channel_des", p_channel);
            json_p.add_child("sequence", p_sequence);

            probe_list.push_back(std::make_pair("", json_p));

            biall_ps.emplace(probeset_id);
            json_ps.add_child("probe_list", probe_list);
            probeset_list.push_back(std::make_pair("", json_ps));
        }
    }

    json.add_child("probeset_list", probeset_list);
    boost::property_tree::json_parser::write_json(output + "_probe.json", json);

    std::ofstream biall_out("biallelic_probe.ps");

    biall_out << "probeset_id\n";

    for (auto &probeset : biall_ps)
        biall_out << probeset << "\n";

    return 0;
}