#include <CPT/format/json.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/combine.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <fstream>
#include <string>
#include <iomanip>
#include <numeric>

namespace alg = boost::algorithm;
namespace bfs = boost::filesystem;
namespace bpt = boost::property_tree;

class Basis
{
  public:
    static const int order = 6;
    static auto run(const int x, const int y)
    {
        cv::Mat_<float> phi = (
            decltype(phi)(1, order)
            << x*x, x*y, y*y, x, y, 1
            // << x, y, 1
            // << 1
        );
        return phi;
    }
};

auto basis(const int x, const int y)
{
    cv::Mat_<float> phi = (
        decltype(phi)(1, 6)
        << x*x, x*y, y*y, x, y, 1
    );
    return phi;
}

class Mapper
{
  private:
    std::map<std::tuple<int32_t, int32_t>, int32_t> map;
    std::vector<cv::Point> vec;

  public:
    auto operator()(const int x, const int y)
    {
        auto& index = map[std::make_tuple(x, y)];
        if (index == 0)
        {
            index = map.size();
            vec.emplace_back(x, y);
        }
        return index - 1;
    }
    auto operator()(const int i)
    {
        return vec[i];
    }
    auto& points(void)
    {
        return vec;
    }
};

class Node
{
  public:
    int32_t row;
    int32_t col;
    cv::Mat_<float> t;
    cv::Mat_<float> y;
    cv::Mat_<float> z;
    cv::Mat_<float> w;

  public:
    Node(const int32_t row, const int32_t col)
      : row(row)
      , col(col)
    {}

    bool adjoins(const Node& other) const
    {
        return std::abs(other.row - this->row) <= 1
            && std::abs(other.col - this->col) <= 1;
    }

    void update(
        const cv::Mat_<float>& gradient
      , const cv::Mat_<float>& rho
    ) {
        w += gradient.mul(rho);
    }

    template <class OVLPS>
    static auto score(
        const Node& u
      , const Node& v
      , OVLPS&& ovlps
    ) {
        const int32_t dr = v.row - u.row;
        const int32_t dc = v.col - u.col;
        double s = 0.0;
    
        if (dr != 0 or dc != 0)
        {
            boost::for_each(
                boost::combine(
                    ovlps[1 + dr][1 + dc]
                  , ovlps[1 - dr][1 - dc]
                )
              , [&u, &v, &s](const auto& tuple)
                {
                    const auto i = boost::get<0>(tuple);
                    const auto j = boost::get<1>(tuple);
                    s += (u.z(i) - v.z(j)) * (u.z(i) - v.z(j));
                }
            );
        }
        else // (dx == 0 or dy == 0)
        {
            // boost::for_each(
            //     ovlps[1 + dr][1 + dc]
            //   , [&u, &s](const auto& i)
            //     {
            //         s += (u.z(i) - 1.0) * (u.z(i) - 1.0);
            //     }
            // );
        }
        return s;
    }

    template <class OVLPS>
    static auto gradient(
        const Node& u
      , const Node& v
      , const cv::Mat_<float>& Phi
      , OVLPS&& ovlps
    ) {
        const int32_t dr = v.row - u.row;
        const int32_t dc = v.col - u.col;
        cv::Mat_<float> g(Basis::order, 1, 0.0);
    
        if (dr != 0 or dc != 0)
        {
            boost::for_each(
                boost::combine(
                    ovlps[1 + dr][1 + dc]
                  , ovlps[1 - dr][1 - dc]
                )
              , [&u, &v, &g, &Phi](const auto& tuple)
                {
                    const auto i = boost::get<0>(tuple);
                    const auto j = boost::get<1>(tuple);
                    auto&& scale = u.z(i) * (u.z(i) - v.z(j)) / u.y(i);
                    g += scale * Phi.row(i).t();
                }
            );
        }
        else // (dx == 0 or dy == 0)
        {
            boost::for_each(
                ovlps[1 + dr][1 + dc]
              , [&u, &g, &Phi](const auto& i)
                {
                    auto&& scale = u.z(i) * (u.z(i) - 1.0) / u.y(i);
                    g += scale * Phi.row(i).t();
                }
            );
        }
        return g;
    }
};

int main(int argc, char* argv[])
{
    std::cerr << std::fixed;
    // read configuration
    auto json = cpt::format::read_json(argv[1]);

    // read marker information
    const auto marker_width  = 10;
    const auto marker_height = 10;
    std::vector<cv::Rect> markers;
    {
        auto list = json.get_list("markers");
        std::vector<int> points;
        for (auto& node: list.get_root())
            points.emplace_back(node.second.get_value<int>());
        for (int i = 0; i < points.size(); i += 2)
            markers.emplace_back(
                points[i]
              , points[i + 1]
              , marker_width
              , marker_height
            );
    }

    // read expansions
    std::vector<int> expansions;
    {
        auto list = json.get_list("expanse");
        for (auto& node: list.get_root())
            expansions.emplace_back(node.second.get_value<int>());
    }
    
    // load images
    std::vector<cv::Mat_<float>> images;
    {
        auto list = json.get_list("images");
        for (auto& node: list.get_root())
        {
            auto&& path = node.second.get_value<std::string>();
            std::cerr << path;
            std::ifstream is(path);
            std::string line;
            std::vector<std::string> tokens;
            int32_t num_lines = 0;
            cv::Mat_<float> image;
            while (std::getline(is, line))
            {
                num_lines++;
                boost::iter_split(tokens, line, alg::first_finder("\t"));
                cv::Mat_<float> col;
                for (auto& token: tokens)
                    col.push_back(std::stof(token, nullptr));
                image.push_back(col.reshape(1, 1));
            }
            images.push_back(image.reshape(1, num_lines));
            std::cerr << " : " << image.rows
                      << " x " << image.cols << "\n";
        }
    }
    
    // define constants
    const auto C = json.get<int32_t>("cols");
    const auto R = json.get<int32_t>("rows");
    const auto W = json.get<int32_t>("width");
    const auto H = json.get<int32_t>("height");
    const auto a = marker_width  + expansions[2] + expansions[3];
    const auto b = marker_height + expansions[0] + expansions[1];

    // init ovlps
    std::vector<int> ovlps[3][3];
    Mapper map;
    {
        const auto y01 = b, y12 = H - b;
        const auto x01 = a, x12 = W - a;

        for (auto& m: markers)
        {
            ovlps[1][1].emplace_back(map(m.x, m.y));
        }
        for (int y = 0; y != H; ++y)
        {
            for (int x = 0; x != W; ++x)
            {
                if (x >= x01 and x < x12 and
                    y >= y01 and y < y12)
                    continue;

                auto i = map(x, y);
                if (y < y01) // top (0)
                {
                    if (x < x01) // left (0)
                    {
                        ovlps[0][0].emplace_back(i);
                        ovlps[1][0].emplace_back(i);
                    }
                    else if (x >= x12) // right (2)
                    {
                        ovlps[0][2].emplace_back(i);
                        ovlps[1][2].emplace_back(i);
                    }
                    // center (1)
                    ovlps[0][1].emplace_back(i);
                }
                else if (y >= y12) // bottom (2)
                {
                    if (x < x01) // left (0)
                    {
                        ovlps[2][0].emplace_back(i);
                        ovlps[1][0].emplace_back(i);
                    }
                    else if (x >= x12) // right (2)
                    {
                        ovlps[2][2].emplace_back(i);
                        ovlps[1][2].emplace_back(i);
                    }
                    // center (1)
                    ovlps[2][1].emplace_back(i);
                }
                else // (y >= y01 and y < y12) // middle (1)
                {
                    if (x < x01) // left (0)
                    {
                        ovlps[1][0].emplace_back(i);
                    }
                    else if (x >= x12) // right (2)
                    {
                        ovlps[1][2].emplace_back(i);
                    }
                }
            }
        }
        for (int r = 0; r != 3; ++r)
        {
            for (int c = 0; c != 3; ++c)
            {
                std::sort(
                    ovlps[r][c].begin()
                  , ovlps[r][c].end()
                  , [&map](const auto& i, const auto& j)
                    {
                        const auto& ii = map(i);
                        const auto& jj = map(j);
                        return std::make_pair(ii.y, ii.x)
                             < std::make_pair(jj.y, jj.x);
                    }
                );
            }
        }
    }
    // check ovlps
    // cv::namedWindow("1", CV_WINDOW_NORMAL);
    // cv::namedWindow("2", CV_WINDOW_NORMAL);
    // for (int u = 0; u != images.size(); ++u)
    // {
    //     const auto ux = u % C;
    //     const auto uy = u / C;
    //     for (int v = 0; v != images.size(); ++v)
    //     {
    //         const auto vx = v % C;
    //         const auto vy = v / C;
    //         if (std::abs(ux - vx) > 1 or std::abs(uy - vy) > 1)
    //             continue;

    //         const auto dr = vy - uy;
    //         const auto dc = vx - ux;
    //         
    //         // cv::Mat_<uint16_t> tmp;
    //         // for (auto& i: ovlps[1 + dr][1 + dc])
    //         //     tmp.push_back(static_cast<uint16_t>(images[u](map(i))));
    //         // tmp = tmp.reshape(1, (dr == 0)? H: b);
    //         // cv::resizeWindow("1", tmp.cols * 10, tmp.rows * 10);
    //         // cv::imshow("1", tmp);

    //         // tmp.release();
    //         // for (auto& i: ovlps[1 - dr][1 - dc])
    //         //     tmp.push_back(static_cast<uint16_t>(images[v](map(i))));
    //         // tmp = tmp.reshape(1, (dr == 0)? H: b);
    //         // cv::resizeWindow("2", tmp.cols * 10, tmp.rows * 10);
    //         // cv::imshow("2", tmp);

    //         double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0, n = 0;
    //         boost::for_each(
    //             boost::combine(
    //                 ovlps[1 + dr][1 + dc]
    //               , ovlps[1 - dr][1 - dc]
    //             )
    //           , [&map, &x = images[u], &y = images[v], &sx, &sy, &sxx, &sxy, &syy, &n]
    //             (const auto& tuple)
    //             {
    //                 const auto& xi = x(map(boost::get<0>(tuple)));
    //                 const auto& yi = y(map(boost::get<1>(tuple)));
    //                 sx  += xi;
    //                 sy  += yi;
    //                 sxx += xi * xi;
    //                 sxy += xi * yi;
    //                 syy += yi * yi;
    //                 n   += 1.0;
    //             }
    //         );
    //         double r = ( n * sxy - sx * sy )
    //                  / sqrt( n * sxx - sx * sx )
    //                  / sqrt( n * syy - sy * sy );
    //         std::cerr << "CORR = " << r << '\n';

    //         // cv::waitKey(0);
    //     }
    // }

    // init Phi_train
    cv::Mat_<float> Phi_train;
    for (auto& p: map.points())
        Phi_train.push_back(Basis::run(p.x, p.y));
   
    // init Phin_infer
    cv::Mat_<float> Phi_infer;
    for (int y = 0; y != H; ++y)
        for (int x = 0; x != W; ++x)
            Phi_infer.push_back(Basis::run(x, y));
     
    // init nodes
    std::vector<Node> nodes;
    for (int u = 0; u != images.size(); ++u)
    {
        auto& image = images[u];
        Node node(u / C, u % C);
        for (auto& loc: map.points())
            node.t.push_back(image(loc));
        cv::solve(
            Phi_train.rowRange(0, markers.size())
          , node.t.rowRange(0, markers.size())
          , node.w
          , cv::DECOMP_NORMAL
        );
        node.y = Phi_train * node.w;
        node.z = node.t / node.y;
        nodes.push_back(std::move(node));        
    }

    // TODO refine between-image correlation
    if (argc > 2)
    {
        {
            double score = 0.0;
            for (auto& u: nodes)
                for (auto& v: nodes)
                    if (u.adjoins(v))
                        score += Node::score(u, v, ovlps);
            std::cerr << "init score = " << score << '\n';
        }
        const auto maxiter = std::atoi(argv[2]);
        const auto rho = std::atof(argv[3]);
        const auto lag = std::stoi(argv[4]);
        cv::Mat_<float> grad(Basis::order, C * R);
        cv::Mat_<float> accu(Basis::order, C * R, 0.0);
        cv::Mat_<float> temp(Basis::order, 1);
        int iter;
        for (iter = 0; iter != maxiter; ++iter)
        {
            // adagrad (1)
            grad = 0;
            for (auto& u: nodes)
            {
                auto i = u.col + u.row * C;
                for (auto& v: nodes)
                    if (u.adjoins(v))
                        grad.col(i) += Node::gradient(u, v, Phi_train, ovlps);
            }
            accu += grad.mul(grad);
            cv::sqrt(accu, temp);
            for (auto& u: nodes)
            {
                auto i = u.col + u.row * C;
                u.update(grad.col(i), rho / temp.col(i));
                u.y = Phi_train * u.w;
                u.z = u.t / u.y;
            }

            // adagrad (2)
            // grad = 0;
            // for (auto& u: nodes)
            // {
            //     auto i = u.col + u.row * C;
            //     for (auto& v: nodes)
            //         if (u.adjoins(v))
            //             grad.col(i) += Node::gradient(u, v, Phi_train, ovlps);
            //     accu.col(i) += grad.col(i).mul(grad.col(i));
            //     cv::sqrt(accu.col(i), temp);
            //     u.update(grad.col(i), rho / temp);
            //     u.y = Phi_train * u.w;
            //     u.z = u.t / u.y;
            // }

            double score = 0.0;
            for (auto& u: nodes)
                for (auto& v: nodes)
                    if (u.adjoins(v))
                        score += Node::score(u, v, ovlps);

            if (iter < 5 or (iter+1) % lag == 0)
                std::cerr << " score = " << std::setprecision(6) << score
                          << " (iter = " << iter+1 << ")\n";
        }
    }

    // Compute CV
    float gmean = 0.0;
    float gstd = 0.0;
    float gprev = 0.0;
    auto gN = 0;
    for (int i = 0; i != nodes.size(); ++i)
    {
        auto& u = nodes[i];
        auto& m = images[i];

        cv::Mat_<float> tmp = Phi_infer * u.w;
        tmp = .5 * m / tmp.reshape(1, W);
        tmp.copyTo(m);
        m *= 65535;

        cv::Mat_<uint8_t> mask;
        tmp.convertTo(mask, mask.type(), 255.0);
        double thres = cv::threshold(
            mask
          , mask
          , 0.0
          , 255.0
          , cv::THRESH_BINARY | cv::THRESH_OTSU
        ) / 255.0;

        auto N = 0;
        auto std = 0.0f;
        auto fg_mean = std::accumulate(
            tmp.begin(), tmp.end(), 0.0f
          , [&N, &std, &thres, &gmean, &gstd, &gprev, &gN]
            (auto& ini, const auto& value)
            {
                if (value > thres)
                {
                    gprev = gmean;
                    gmean += (value - gmean) / ++gN;
                    gstd  += (value - gmean) * (value - gprev);

                    auto prev = ini;
                    ini += (value - ini) / ++N;
                    std += (value - ini) * (value - prev);
                }
                return ini;
            }
        );
        std = std::sqrt(std / (N - 1));
        auto CV = std / fg_mean * 100; 
        std::cerr << "CV = " << std::setprecision(2) << CV << "% "
                  << "(thres = " << thres << ") , ";

        N = 0;
        std = 0.0f;
        auto bg_mean = std::accumulate(
            tmp.begin(), tmp.end(), 0.0f
          , [&N, &std, &thres](auto& ini, const auto& value)
            {
                if (value <= thres)
                {
                    auto prev = ini;
                    ini += (value - ini) / ++N;
                    std += (value - ini) * (value - prev);
                }
                return ini;
            }
        );
        std = std::sqrt(std / (N - 1));
        auto SN = 20.0 * (std::log10(fg_mean) - std::log10(bg_mean));
        std::cerr << "SN = " << SN << " db , w" << nodes[i].w.t() << '\n';

        const auto name = std::to_string(i);
        cv::namedWindow(name, ::CV_WINDOW_NORMAL);
        cv::resizeWindow(name, W * 5, H * 5);
        cv::imshow(name, tmp);
    }
    gstd = std::sqrt(gstd / (gN - 1));
    std::cerr << "total CV = " << std::setprecision(2) << gstd / gmean * 100 << "%\n";


    // Compute correlation
    for (int u = 0; u != images.size(); ++u)
    {
        const auto& ux = u % C;
        const auto& uy = u / C;
        const auto& x = images[u];

        for (int v = 0; v != images.size(); ++v)
        {
            const auto& vx = v % C;
            const auto& vy = v / C;
            const auto& y = images[v];
            if (u == v)
                continue;

            if (std::abs(ux - vx) <= 1 and std::abs(uy - vy) <= 1)
            {
                const auto& dr = vy - uy;
                const auto& dc = vx - ux;
                double total = 0;
                double lse = 0;
                boost::for_each(
                    boost::combine(
                        ovlps[1 + dr][1 + dc]
                      , ovlps[1 - dr][1 - dc]
                    )
                  , [&x, &y, &lse, &total, &map](const auto& tuple)
                    {
                        const auto& val1 = x(map(boost::get<0>(tuple)));
                        const auto& val2 = y(map(boost::get<1>(tuple)));
                        total += 1;
                        lse += ((val1 - val2) * (val1 - val2) - lse) / total;
                    }
                );
                lse = std::sqrt(lse);

                // double sx = 0, sy = 0, n = 0;
                // double sxx = 0, sxy = 0, syy = 0;
                // boost::for_each(
                //     boost::combine(
                //         ovlps[1 + dr][1 + dc]
                //       , ovlps[1 - dr][1 - dc]
                //     )
                //   , [&x, &y, &n, &sx, &sy, &sxx, &sxy, &syy, &map](const auto& tuple)
                //     {
                //         const auto xi = x(map(boost::get<0>(tuple)));
                //         const auto yi = y(map(boost::get<1>(tuple)));
                //         sx  += xi;
                //         sy  += yi;
                //         sxx += xi * xi;
                //         sxy += xi * yi;
                //         syy += yi * yi;
                //         n   += 1.0;
                //     }
                // );
                // double r = ( n * sxy - sx * sy )
                //          / std::sqrt( n * sxx - sx * sx )
                //          / std::sqrt( n * syy - sy * sy );
                
                std::cerr << "CORR( " << std::setw(2) << u
                          << " -> "   << std::setw(2) << v
                          << " ) = "  << std::setprecision(6) << lse << '\n';
            }
        }
    }

//    for (int i = 0; i != images.size(); ++i)
//    {
//        cv::namedWindow(std::to_string(i), ::CV_WINDOW_NORMAL);
//        cv::resizeWindow(std::to_string(i), 500, 500);
//    }
//
//    for (auto& u: nodes)
//    {
//        std::cerr << u.row << ", " << u.col << "\t" << u.t.total() << '\n';
//    }
//    for (int i = 0; i != images.size(); ++i)
//    {
//        const auto& image = images[i];
//        const auto& node = nodes[i];
//    }

//    cv::Mat_<float> g(6, 1);
//    const int maxiter = 8000;
//    for (int iter = 0; iter != maxiter; ++iter)
//    {
//        std::cerr << '\n';
//        for (auto& u: nodes)
//        {
//            g = 0;
//            for (auto& v: nodes)
//                if (u.adjoins(v))
//                    g += Node::gradient(u, v);
//
//            std::cerr << g.t() << '\n';
//
//            u.update(g, 1e-8);
//            u.eval();
//        }
//        //for (int i = 0; i != images.size(); ++i)
//        //{
//        //    cv::Mat_<float> res = nodes[i].calib(images[i].clone());
//        //    // double maxval;
//        //    // cv::minMaxLoc(res, 0, &maxval);
//        //    // std::cerr << maxval << '\n';
//        //    // cv::imshow(std::to_string(i), res);
//        //}
//    }

    return 0;
}
