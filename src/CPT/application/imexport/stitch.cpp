#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

struct MultipleInputPaths
{
    using Path = boost::filesystem::path;

    void operator()(const std::vector<Path>& paths)
    {
        bool terminate = false;
        for (const auto& path: paths)
        {
            std::cerr << "+ " << path.string();
            if (!boost::filesystem::exists(path))
            {
                std::cerr << " not found!";
                terminate = true;
            }
            std::cerr << '\n';
        }
        if (terminate)
        {
            throw std::invalid_argument
            ("Some input paths are invalid. Abort!");
        }
    }
};

int main(int argc, char* argv[])
{
    namespace bpo = boost::program_options;
    namespace bfs = boost::filesystem;
    
    std::vector<int> dims { 3, 3 };
    std::vector<int> locs { 1, 2, 3, 6, 5, 4, 7, 8, 9 }; // 1-based
    std::vector<bfs::path> input_paths;
    bfs::path output_path;

    bpo::options_description desc("Allowed ptions");
    desc.add_options()

        ( "help,h", "show this help messages")
        ( "images,i", bpo::value(&input_paths)->required()
                      ->multitoken()->notifier(MultipleInputPaths())
        , "file path to input data" )
        ( "outdir,o", bpo::value(&output_path)->default_value(".")
        , "output directory" )
        ( "dim,d", bpo::value(&dims)->multitoken()
        , "input dimension of data matrix" )
        ( "loc,x", bpo::value(&locs)->multitoken()
        , "input location of data matrix" )
    ;
    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);

    if (argc == 1 or vm.count("help"))
    {
        std::cerr << desc << "\n";
        return 1;
    }
    bpo::notify(vm);

    if (dims[0] * dims[1] 


    return 0;
}
