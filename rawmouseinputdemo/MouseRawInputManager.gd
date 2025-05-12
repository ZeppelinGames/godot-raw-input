extends Node2D

func _ready():
	var raw_input = RawMouseInput.new();
	
	# Needs to be deferred 
	raw_input.call_deferred("init");
	add_child(raw_input);
	
	var callable :Callable = Callable(self, "_on_raw_mouse");
	raw_input.connect("raw_mouse", callable)
	
func _on_raw_mouse(usFlags, ulButtons, usButtonFlags, usButtonData, ulRawButtons, dx, dy):
	print("Mouse moved: dx = %d, dy = %d" % [dx, dy])
