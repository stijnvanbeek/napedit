#include "imguifunctions.h"

#include "imgui_internal.h"

namespace nap
{

	namespace edit
	{

		bool TreeNodeArrow(const char *label, bool defaultOpen)
		{
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
			if (defaultOpen)
				flags |= ImGuiTreeNodeFlags_DefaultOpen;
			bool opened = ImGui::TreeNodeEx(label, flags);
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			return opened;
		}


		bool Selectable(const char *label, bool selected, float width)
		{
			bool result = false;
			float height = ImGui::GetFrameHeight();
			result = ImGui::Selectable(label, selected, ImGuiSelectableFlags_None, ImVec2(width, height)) || ImGui::IsItemClicked(1);
			return result;
		}


		void Icon(Texture2D &image, IMGuiService *service)
		{
			float size = 17.f * service->getScale();
			ImGui::Image(service->getTextureHandle(image), ImVec2(size, size));
		}

	}

}