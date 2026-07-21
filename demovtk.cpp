// demovtk.cpp - cubo rotando. Build: ver CMakeLists.txt abajo.
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCubeSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

// Timer dispara este callback ~60x/s: gira actor y re-renderiza.
static void RotateCB(vtkObject* caller, unsigned long, void* clientData, void*)
{
  auto* iren = static_cast<vtkRenderWindowInteractor*>(caller);
  auto* actor = static_cast<vtkActor*>(clientData);
  actor->RotateY(1.0);
  actor->RotateX(0.5);
  iren->GetRenderWindow()->Render();
}

int main()
{
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkCubeSource> cube;  // lado 1 centrado en origen por defecto

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(cube->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

  vtkNew<vtkRenderWindow> window;
  window->AddRenderer(renderer);
  window->SetSize(600, 600);
  window->SetWindowName("Cubo rotando");

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(window);

  window->Render();
  iren->Initialize();

  vtkNew<vtkCallbackCommand> cb;
  cb->SetCallback(RotateCB);
  cb->SetClientData(actor);
  iren->AddObserver(vtkCommand::TimerEvent, cb);
  iren->CreateRepeatingTimer(16);  // ms -> ~60 fps

  iren->Start();
  return 0;
}
