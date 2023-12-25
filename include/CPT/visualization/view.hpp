#pragma once

#include <CPT/visualization/vtk_util.hpp>
#include <Nucleona/type_traits/core.hpp>
#include <boost/proto/traits.hpp>
namespace cpt {
namespace visualization {

namespace cv = cpt::visualization;
namespace bp = boost::proto;

class View
{
  public:
    std::size_t height { 800 };
    std::size_t width  { 600 };
    cv::VtkContextViewPtr view { cv::VtkContextViewPtr::New() };
    auto prepare_view( )
    {
        view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
        view->GetRenderWindow()->SetSize(height, width);
        view->GetRenderWindow()->SetPosition(1000, 500);
    }
    auto image_dump(const std::string& fname)
    {
        // view->GetRenderWindow()->OffScreenRenderingOn();
        view->GetRenderWindow()->Render();
        auto w2if ( cv::VtkWindowToImageFilterPtr::New() );
        w2if->SetInput( view->GetRenderWindow() );
        w2if->SetMagnification(1);
        w2if->SetInputBufferTypeToRGBA();
        w2if->ReadFrontBufferOff();
        w2if->Update();

        auto writer = cv::VtkPNGWriterPtr::New();
        writer->SetFileName(fname.c_str());
        writer->SetInputConnection(w2if->GetOutputPort());
        writer->Write();
    }
    auto render_and_start( )
    {
        view->GetRenderWindow()->SetMultiSamples(0);
        view->GetInteractor()->Initialize();
        view->GetInteractor()->Start();
    }
    template<class T>
    std::enable_if_t<
        ::decay_equiv_v<T, cv::VtkTablePtr>
        , T
    > to_table( T&& dataset )
    {
        return std::forward<T>(dataset);
    }
};


}}
