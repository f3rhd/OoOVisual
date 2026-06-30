// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <Visualizer/App.h>
#include <Visualizer/Constants.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		GLFWwindow* App::_window{ nullptr };
		bool App::init() {
			if (!glfwInit()) return false;

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			_window = glfwCreateWindow(Constants::INITIAL_WINDOW_WIDTH, Constants::INITIAL_WINDOW_HEIGHT, "OoO CPU Visualizer", nullptr, nullptr);
			if (!_window) {
				glfwTerminate();
				return false;
			}

			glfwMakeContextCurrent(_window);
			glfwSwapInterval(1);

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			ImGui::StyleColorsDark();

			ImGui_ImplGlfw_InitForOpenGL(_window, true);
			ImGui_ImplOpenGL3_Init("#version 330");

			return true;
		}

		void App::start_frame() {
			glfwPollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::SetNextWindowSize(window_size());
		}

		void App::end_frame() {
			ImGui::Render();
			int display_w{}, display_h{};
			glfwGetFramebufferSize(_window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);

			glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(_window);
		}

		bool App::should_close() {
			return glfwWindowShouldClose(_window);
		}
		void App::close() {
			glfwSetWindowShouldClose(_window, 1);
		}
		void App::cleanup(Scene& scene) {
			scene.clear();
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			glfwDestroyWindow(_window);
			glfwTerminate();
		}

		ImVec2 App::window_size() {

			int width{};
			int height{};
			glfwGetWindowSize(_window, &width, &height);
			return ImVec2(static_cast<float>(width), static_cast<float>(height));
		}

	}
}