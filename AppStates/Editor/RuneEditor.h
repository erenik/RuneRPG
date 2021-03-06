// Emil Hedemalm
// 2013-06-28

#ifndef RUNE_EDITOR_H
#define RUNE_EDITOR_H

#include "AppStates/AppState.h"
#include "AppStates/AppStates.h"
#include "Entity/Entities.h"

class GridObject;
class GridObjectType;
class Camera;
class TileMap2D;
class Script;
class Light;
struct TileType;
class GraphicsState;
class AppWindow;

namespace EditMode {
enum editModes{
	NULL_MODE,
	SIZE, 
	TILES, 
	TERRAIN, 
	OBJECTS, 
	EVENTS,
	LIGHTING,
	EDIT_MODES,
};};
	

class RuneEditor : public AppState{
public:
	RuneEditor();
	/// Virtual destructor to discard everything appropriately.
	virtual ~RuneEditor();
	void OnEnter(AppState * previousState);
	void Process(int timeInMs);
	void OnExit(AppState * nextState);
	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	///
	virtual Entities GetActiveSelection() { return runeEditorSelection; };

	/// Called every time the current selection is updated.
	void OnSelectionUpdated();

	/// Input functions for the various states
	void MouseClick(AppWindow * AppWindow, bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(AppWindow * AppWindow, bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseMove(AppWindow * AppWindow, int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(AppWindow * AppWindow, float delta);
	/// Callback from the Input-manager, query it for additional information as needed.
	void KeyPressed(int keyCode, bool downBefore);

	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	void Render(GraphicsState & graphicsState);


	/// Sets appropriate variables and updates relevant GUI.
	void SetBrushType(int type);
	void SetBrushSize(int size);

private:
	/// Opens the brush editor AppWindow.
	void OpenSizeEditor();
	void OpenBrushEditor();
	void OpenTileSelector();
	void OpenTerrainSelector();
	void OpenObjectSelector();
	void OpenLightingEditor();
	void HideEditorWindows();

	/// Attempts to delete object at current cursor position.
	void DeleteObject();

	/// Saves actively edited object, pushing it to list of all objects. calls OnObjectTypesUpdated on success.
	void SaveObjects();
	/// Updates list of available objects.
	void OnObjectTypesUpdated();
	/// Called once the edit ui for objects has been pushed.
	void OnObjectTypeSelectedForEditing();
			
	/// Called to update ui.
	void OnTileTypesUpdated();
	void OnTileTypeSelected();

	/// For the edit modes.
	String GetModeName(int id);

	// Used to keep track of saving for when query-dialogues are necessary.
	String mapFilePath;
	String rootMapDir;
	/// Saves the map to set mapFilePath. Assumes any file-checks have been done beforehand. Pauses input and physics while saving.
	bool SaveMap();
	/// Attempts to load target map
	bool LoadMap(String name);

	void SetMapSize(int x, int y);

	void UpdateUISize();
	void UpdateUITiles();
	void UpdateUIObjects();
	void UpdateUILighting();

	/// Update GUI
	void OnEditModeUpdated(int previousMode);
	// Wosh
	void OnSelectedEventUpdated();
	/// Update more gui.
	void OnSelectedLightUpdated();
	/// Queries the TileTypeManager to reload the available tiles.
	void ReloadTiles();
	/// Fetches active/relevant set of tiles that will be painted if wished so now. Uses saves mouse coordinate-data to calculate the set.
	List<Vector2i> GetTilesToPaint(int brushType, int brushSize);

	/// Active map.
	TileMap2D * map;
	Script * selectedEvent;
	Light * selectedLight;
	/// New size to apply to map.
	Vector2i newSize;
	
	/// Grid object type. For creating new objects that can be placed on the map.
	GridObjectType * objectType;

	/// In 3D-world
	Vector3f cursorPosition, previousCursorPosition;
	/// Float values for painting smoothly!
	Vector3f cursorPositionPreRounding;

	enum brushType{
		SQUARE, CIRCLE, DRAG_RECT, DRAG_CIRCLE, MAX_BRUSH_TYPES,
	};
	// Which mode we're in..
	int editMode;
	String modeName;
	/// For painting!
	int brushType, brushSize;
	/// For painting tiles o-o'
	int tileTypeIndex, tileBrushSize, tileBrushType;
	TileType * tileType;
	int terrainTypeSelected, terrainBrushSize, terrainBrushType;


	/// Transitions to the level test state!
	void Playtest();
	/// Paint!
	void Paint();
	/// Attempts to create an object on current mouse location. Returns object if it succeeded.
	GridObject * CreateObject();
	/// SElect evvveettttnt, using given mouse coordinates. Returns false if none was close enough.
	bool SelectEvent();
	/// Create an event on the map! :)
	void CreateEvent();
	/// Attempts to delete an event at given location!
	void DeleteEvent();
	/// Tries to select light at current cursor position.
	bool SelectLight();
	/// Creates a new light
	void CreateLight();
	void DeleteLight();

	/// Set defaults!
	void ResetCamera();
	/// The RuneEditor camera! :D
	Camera * runeEditorCamera;
	/// Actively manipulated entities
	Entities runeEditorSelection;

	/// Mouse states
	bool lButtonDown;
	bool rButtonDown;
	/// Starting positions for when dragging mouse for selection, etc.
	int startMouseX, startMouseY;
	int mouseX, mouseY;


	/// For setting brush type and size. Used for painting tiles and terrain, but later on maybe objects as well?
	AppWindow * sizeEditor;
	AppWindow * brushEditor;
	AppWindow * tileSelector;
	AppWindow * terrainSelector;
	AppWindow * objectSelector;
	AppWindow * lightingEditor;
	/// List of all windows, for eased manipulation.
	List<AppWindow*> editorWindows;

	enum mouseCameraStates {
		NULL_STATE,
		ROTATING,
		PANNING
	};
	int mouseCameraState;
};

#endif
