import unreal


ASSET_PATH = "/Game/Gaters/Materials/M_TerrainVertexColor"


def create_material():
    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        unreal.EditorAssetLibrary.delete_asset(ASSET_PATH)

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_TerrainVertexColor",
        "/Game/Gaters/Materials",
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
    if material is None:
        raise RuntimeError(f"Could not create {ASSET_PATH}")

    vertex_color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVertexColor, -300, 0
    )
    if not unreal.MaterialEditingLibrary.connect_material_property(
        vertex_color, "", unreal.MaterialProperty.MP_BASE_COLOR
    ):
        raise RuntimeError("Could not connect vertex color to Base Color")

    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -300, 150
    )
    roughness.set_editor_property("r", 0.82)
    if not unreal.MaterialEditingLibrary.connect_material_property(
        roughness, "", unreal.MaterialProperty.MP_ROUGHNESS
    ):
        raise RuntimeError("Could not connect terrain roughness")

    specular = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -300, 300
    )
    specular.set_editor_property("r", 0.2)
    if not unreal.MaterialEditingLibrary.connect_material_property(
        specular, "", unreal.MaterialProperty.MP_SPECULAR
    ):
        raise RuntimeError("Could not connect terrain specular")

    unreal.MaterialEditingLibrary.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_asset(ASSET_PATH, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {ASSET_PATH}")
    unreal.log(f"Created default-lit, non-emissive terrain material: {ASSET_PATH}")


create_material()
