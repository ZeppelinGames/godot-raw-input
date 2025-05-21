extends Node

signal raw_mouse(guid, usFlags, ulButtons, usButtonFlags, usButtonData, ulRawButtons, dx, dy);
signal raw_mouse_init();

var rawMouseInput : RawMouseInput = null;

func _ready():
	print('Attempting init')
	call_deferred("_rawmouse_init");
	
func _rawmouse_init():
	if rawMouseInput == null:
		print('Initialising RawMouseInput')
		rawMouseInput = RawMouseInput.new()
		add_child(rawMouseInput);
		rawMouseInput.connect("raw_mouse", Callable(self, "_on_internal_raw_mouse"));
		emit_signal("raw_mouse_init");
		
func _on_internal_raw_mouse(guid, usFlags, ulButtons, usButtonFlags, usButtonData, ulRawButtons, dx, dy):
	emit_signal("raw_mouse", guid, usFlags, ulButtons, usButtonFlags, usButtonData, ulRawButtons, dx, dy);
