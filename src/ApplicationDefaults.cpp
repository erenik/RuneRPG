/// Emil Hedemalm
/// 2014-04-06
/// Sets default values for this project, like directories.

#include "Application/Application.h"

#include "Maps/MapManager.h"
#include "Graphics/Fonts/TextFont.h"
#include "Script/Script.h"
#include "UI/Input/ILabelInput.h"
#include "UI/UserInterface.h"
#include "UI/UIButtons.h"
#include "UI/Lists/UIScrollBarHandle.h"
#include "UI/UI.h"
#include "UI/Lists/UIScrollBar.h"
#include "UI/UIFileBrowser.h"
#include "UI/Input/UIDropDownMenu.h"

void SetApplicationDefaults()
{
	const String UITexturesDir = "img/ui/";

	ILabelInput::defaultInput3Slice = "img/ui/3Slice_Input_Bg";
	ILabelInput::defaultInputOnHover3Slice = "img/ui/3Slice_Input_Bg_Active";

	UI::defaultAcceptMathematicalExpressions = false;

	UIVisuals::onIdleHighlightFactor = 0.0f;
	UIVisuals::onHoverHighlightFactor = 0.15f;
	UIVisuals::onActiveHightlightFactor = 0.20f;
	UIVisuals::onToggledHighlightFactor = 0.2f;

	UIVisuals::scrollBarTemplate = "gui/Components/ScrollBar.gui";
	UIVisuals::scrollBarHandleTemplate = "gui/Components/ScrollBarHandle.gui";

	UIScrollBar::default3SliceTexture = UITexturesDir + "ScrollBarBG";
	UIScrollBarHandle::default3SliceTexture = UITexturesDir + "ScrollBarHandle";
	UIScrollBarHandle::default3SliceOnHover = UITexturesDir + "ScrollBarHandle_Active";
	UIScrollBarHandle::defaultSmallestTexture = UITexturesDir + "SmallScrollBar";

	UIButton::defaultThreeSlice = UITexturesDir + "3SliceButton.png";
	UIButton::defaultThreeSliceOnHover = UITexturesDir + "3SliceButton_Active.png";

	UIDropDownMenu::default3Slice = UITexturesDir + "3SliceDropdown.png";
	UIDropDownMenu::default3SliceOnHover = UITexturesDir + "3SliceDropdown_Active.png";

	UIFileBrowser::defaultFileBrowserSrc = "gui/Editor/FileBrowser.gui";
	UIFileBrowser::defaultFileButtonSrc = "gui/Editor/FileEntry.gui";

	Application::name = "xRPG";
//	TextFont::defaultFontSource = "font3";
	TextFont::defaultFontSource = "";
	UserInterface::rootUIDir = "gui/";
	/// This should correspond to the directory name of the released application's final install.
	FilePath::workingDirectory = "RuneRPG";
	MapManager::rootMapDir = "map/";
	UIVisuals::defaultTextureSource = "";
	/*
	*/
}