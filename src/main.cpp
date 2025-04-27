#include "Definition.h"
#include "Render/Renderer.h"
#include "Render/Camera.h"
#include "PhysicsRules.h"


int main() {
	Camera camera;
	camera.SetPosition(Vector3(0.0, 0.0, 5.0f));
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

	Integer frameIndex = 0;
	PhysicModel model("Config/cloud.json");
	//model.GetParticleSystem().WriteToFile("Data\Frame" + std::to_string(frameIndex++) + ".ply");

	while (rend.IsRendering()) {
		rend.ClearScreen();
		glfwPollEvents();


		model.Step(0.01);

		Matrix4 mvpMatrix = camera.GetMVPMatrix(identityTransform);
		rend.RenderParticles(model.GetParticleSystem().GetParticles(), mvpMatrix, 30.0);
		rend.RenderLines(points, indices, mvpMatrix, 5.0, Color::Black());
		//model.GetParticleSystem().WriteToFile("Data/Frame" + std::to_string(frameIndex++) + ".ply");
		//if (frameIndex >= 100) break;

		rend.LoadScreen();
	}
}
