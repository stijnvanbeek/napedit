#pragma once

#include <imgui.h>
#include <imguiservice.h>

namespace nap
{

	namespace edit
	{

		bool TreeNodeArrow(const char* label);
		bool Selectable(const char* label, bool selected, float width);
		void Icon(Texture2D& image, IMGuiService* service);;

	}

}