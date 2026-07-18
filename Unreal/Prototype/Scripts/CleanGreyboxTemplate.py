import unreal


MAP = "/Game/Gaters/Maps/Lvl_GateGreybox"
TEMPLATE_MESH_PREFIX = "/Game/LevelPrototyping/"
TEMPLATE_FLOOR = "/Engine/MapTemplates/SM_Template_Map_Floor.SM_Template_Map_Floor"
REQUIRED_ACTORS = {"DirectionalLight", "PlayerStart", "PostProcessVolume", "SkyLight"}


def mesh_paths(actor):
    result = []
    for component in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = component.get_editor_property("static_mesh")
        if mesh:
            result.append(mesh.get_path_name())
    return result


def is_template_scenery(actor):
    return any(
        path.startswith(TEMPLATE_MESH_PREFIX) or path == TEMPLATE_FLOOR
        for path in mesh_paths(actor)
    )


unreal.EditorLoadingAndSavingUtils.load_map(MAP)
actors = unreal.EditorLevelLibrary.get_all_level_actors()
template_actors = [actor for actor in actors if is_template_scenery(actor)]

if "-CleanGreyboxTemplate" in unreal.SystemLibrary.get_command_line():
    labels = sorted(actor.get_actor_label() for actor in template_actors)
    unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actors(template_actors)
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Could not save {}".format(MAP))
    unreal.log("GREYBOX_TEMPLATE_REMOVED count={} labels={}".format(len(labels), ",".join(labels)))
else:
    present_labels = {actor.get_actor_label() for actor in actors}
    missing_required = sorted(REQUIRED_ACTORS - present_labels)
    if missing_required:
        raise RuntimeError("Required level actors missing: {}".format(",".join(missing_required)))
    if template_actors:
        raise RuntimeError(
            "Template scenery remains: {}".format(
                ",".join(sorted(actor.get_actor_label() for actor in template_actors))
            )
        )
    unreal.log("GREYBOX_TEMPLATE_VERIFY PASS required={} template=0".format(len(REQUIRED_ACTORS)))
