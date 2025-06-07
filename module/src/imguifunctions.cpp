#include "imguifunctions.h"

#include "imgui_internal.h"

namespace nap
{

	namespace edit
	{

		bool TreeNodeArrow(const char *label)
		{
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.f, 0.f, 0.f, 0.f));
			bool opened = ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			return opened;
		}


		bool Selectable(const char *label, bool selected, float width)
		{
			bool result = false;
			result = ImGui::Selectable(label, selected, ImGuiSelectableFlags_None, ImVec2(width, ImGui::GetItemRectSize().y)) || ImGui::IsItemClicked(1);
			return result;
		}
	}

}