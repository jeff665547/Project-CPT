#include <iostream>
#include <fstream>
#include <map>
#include <CPT/format/cen.hpp>
#include <Nucleona/format/hdf5.hpp>
#include <CFU/format/cen/mask.hpp>
#include <CFU/format/cen/outlier.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        std::cout << "cenfile_channel_reorder [cen files list] [order_0,order1]" << std::endl;
        return 1;
    }

    auto cen_files = std::string(argv[1]);
    auto order_str = std::string(argv[2]);

    auto file_in = std::ifstream(cen_files);
    auto split_vec = std::vector<std::string>();
    auto cenfile_path = std::string();

    auto order_vec = std::vector<std::string>();
    auto order_map = std::map<std::string, std::size_t>();
    boost::iter_split(order_vec, order_str, boost::algorithm::first_finder(","));

    while (std::getline(file_in, cenfile_path))
    {
        if (cenfile_path.empty())
            continue;

        boost::iter_split(split_vec, cenfile_path, boost::algorithm::first_finder("."));
        auto paht_name = split_vec[0];

        cfu::format::cen::File file(cenfile_path);
        nucleona::format::hdf5::File fixd(paht_name + "_fix.cen", H5F_ACC_TRUNC);

        auto data = file.extract_data();
        auto root = fixd.open_group("/");
        
        root.create_attribute_v("magic", 67);
        root.create_attribute_v("version", std::uint8_t(2));

        auto array(root.create_group("array"));

        array.create_attribute_v("date", data.date());
        array.create_attribute_v("type", data.type());
        array.create_attribute_v("barcode", data.barcode());
        array.create_attribute_v("feature-columns", data.feature_columns());
        array.create_attribute_v("feature-rows", data.feature_rows());
        array.create_attribute_v("feature-height", data.feature_height());
        array.create_attribute_v("feature-width", data.feature_width());

        auto channel_names = std::vector<std::string>();
        auto data_group_num(data.channels().size());

        for (std::size_t i(0); i < data_group_num; i++)
        {
            auto &cs_channel(data.channels().at(i));
            std::string channel_name(cs_channel.name);

            for (std::size_t j(0); j < order_vec.size(); j++)
            {
                if (channel_name == order_vec[j])
                {
                    order_map[order_vec[j]] = i;
                    break;
                }
            }
        }

        for (std::size_t i(0); i < data_group_num; i++)
        {
            auto &cs_channel(data.channels().at(order_map[order_vec[i]]));
            std::string channel_name(cs_channel.name);
            channel_names.push_back(channel_name);

            auto channel(array.create_group(channel_name));
            channel.create_dataset_v("intensity", cs_channel.intensity);

            channel.create_dataset_v("mask", cfu::format::cen::to_mask_vec(cs_channel.mask));
            channel.create_dataset_v("outlier", cfu::format::cen::to_outlier_vec(cs_channel.outlier));

            channel.create_dataset_v("pixel", cs_channel.pixel);
            channel.create_dataset_v("stddev", cs_channel.stddev);
        }

        array.create_attribute_v("channel-names", channel_names);
    }

    return 0;
}