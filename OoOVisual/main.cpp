// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/App.h>
#include <Visualizer/Scene.h>
#include <Core/Core.h>
using namespace OoOVisual;
int main(int argc, char** argv) {
	if (!Visualizer::App::init())
		return -1;
	if (!Core::init(argc, argv))
		return -1;
	Visualizer::Scene scene{};
	scene.init();
	while (!Visualizer::App::should_close()) {
		Core::run();
		Visualizer::App::start_frame();
		scene.play();
		Visualizer::App::end_frame();
	}
	Visualizer::App::cleanup(scene);
}