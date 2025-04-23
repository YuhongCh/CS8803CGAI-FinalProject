#include "Definition.h"
#include "Render/Renderer.h"
#include "Render/Camera.h"


int main() {
	Camera camera;
	camera.SetPerspective(45.0f, 1.0f, 0.1f, 100.0f);
	Transform identityTransform;

	Renderer& rend = Renderer::GetInstance();
	rend.InstantiateWindow("OpenGL Window", 480, 480);
	rend.SetBackground(Color::White());
	rend.SetUserPointer(camera);

	rend.SetKeyboardCallback(Camera::KeyBoardPressedListener);


#ifdef _DEBUG
	std::vector<Vector3> points = { Vector3(-0.9, -0.9, -0.9), Vector3(-0.9, -0.9, 0.9), Vector3(-0.9, 0.9, -0.9), Vector3(-0.9, 0.9, 0.9),
									Vector3(0.9, -0.9, -0.9), Vector3(0.9, -0.9, 0.9), Vector3(0.9, 0.9, -0.9), Vector3(0.9, 0.9, 0.9) };
	std::vector<Integer> indices = { 0, 1, 1, 5, 5, 4, 4, 0, 2, 3, 3, 7, 7, 6, 6, 2, 0, 2, 1, 3, 4, 6, 5, 7 };
#endif


	while (rend.IsRendering()) {
		rend.ClearScreen();
		glfwPollEvents();

		Matrix4 mvpMatrix = camera.GetMVPMatrix(identityTransform);
		rend.RenderLines(points, indices, mvpMatrix, 5.0, Color::Black());

		rend.LoadScreen();
	}
}
