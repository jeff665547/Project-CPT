#pragma once
#include <vtkSmartPointer.h>
#include <vtkChartXY.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkLongLongArray.h>
#include <vtkShortArray.h>
#include <vtkSignedCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkUnsignedLongLongArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkPlotPoints.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <vtkStringArray.h>
#include <vtkDoubleArray.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#define VTK_POINTER( sym ) \
using Vtk ## sym ## Ptr = vtkSmartPointer<vtk ## sym >
namespace cpt {
namespace visualization {

VTK_POINTER(ContextView             );
VTK_POINTER(ChartXY                 );
VTK_POINTER(Table                   );
VTK_POINTER(FloatArray              );
VTK_POINTER(IntArray                );
VTK_POINTER(LongArray               );
VTK_POINTER(LongLongArray           );
VTK_POINTER(ShortArray              );
VTK_POINTER(SignedCharArray         );
VTK_POINTER(UnsignedCharArray       );
VTK_POINTER(UnsignedIntArray        );
VTK_POINTER(UnsignedLongArray       );
VTK_POINTER(UnsignedLongLongArray   );
VTK_POINTER(UnsignedShortArray      );
VTK_POINTER(StringArray             );
VTK_POINTER(DoubleArray             );
VTK_POINTER(WindowToImageFilter     );
VTK_POINTER(PNGWriter               );

}}
