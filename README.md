# SaveGameableDoOnce
A plugin for UnrealEngine,provides a blueprint node like "Do Once",but can be saved
>Engine Version: 4.26-4.27;5.0-5.1
## Examples
- ExampleMap : SaveGameableDoOnce/Content/Maps/SaveGameableDoOnce
- ExampleBPs : SaveGameableDoOnce/Content/Example/Blueprints
- Save&LoadGame,DeBug : SaveGameableDoOnce/Content/Example/Blueprints/BP_SGDO_Player
## Background
- Our team is developing a first person story game,there are many "Do Once" in our project for some triggers to call events,play sequences and jumpscares...We didn't realize how annoying it is until we start to bulid our save game system.
- Native "Do Once" node doesn't support save game,if we want to save these "Do Once",we need to create a boolean for each of them and replace them with branches,and,traverse all of them when saving,initialize them after loading.Or,serilize these objects.
- So I developed this plugin,and finished our save game system by simply replacing the blueprint of "Do Once" macro in engine:-)
## Limitations
### LevelScripts
- If you changed the level blueprint,Do Once State of level blueprint will lost.Beacuse the reference to level script object changed.
- May be you can follow the way how player's Do Once State saved.
