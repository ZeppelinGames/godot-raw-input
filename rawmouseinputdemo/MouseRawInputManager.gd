extends Control

@onready var outputText := $RichTextLabel;

func _init():
	RawMouseInputWrapper.connect("raw_mouse", Callable(self, "_on_raw_mouse"));

func _on_raw_mouse(deviceName, usFlags, ulButtons, usButtonFlags, usButtonData, ulRawButtons, dx, dy):
	outputText.text = "Mouse (" + deviceName + ") moved: (" + str(dx) + ", " + str(dy) + ")";
	
