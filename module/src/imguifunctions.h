#pragma once

#include <imgui.h>
#include <imguiservice.h>

namespace nap
{

	namespace edit
	{
		/**
		 * Display tree node arrow
		 * @param label Unique label for the widget for internal use by ImGui.
		 * @param defaultOpen True when the tree node is opened by default
		 * @return True when the tree node is opened
		 */
		bool TreeNodeArrow(const char* label, bool defaultOpen = false);

		/**
		 * Display a selectable item with a label
		 * @param label Label text
		 * @param selected True when the item is selected
		 * @param width Width of the item
		 * @return True when the item is selected
		 */
		bool Selectable(const char* label, bool selected, float width);

		/**
		 * Display an icon
		 * @param image Texture to display
		 * @param service IMGuiService to use for rendering
		 */
		void Icon(Texture2D& image, IMGuiService* service);;

	}

}