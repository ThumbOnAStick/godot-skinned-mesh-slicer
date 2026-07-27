extends Node


func _ready() -> void:
	var summator = Summator.new()
	summator.add(3)
	summator.add(4)
	print(summator.get_total())
	
