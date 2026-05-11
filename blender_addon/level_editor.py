import bpy

# Blenderに登録するアドオン情報
bl_info = {
    "name": "レベルエディタ",
    "author": "Yuki Kurata",
    "version": (4, 4),
    "blender": (4, 4, 0),
    "location": "",
    "description": "レベルエディタ",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}

# トップバーの拡張メニュー
class TOPBAR_MT_my_menu(bpy.types.Menu):
    bl_idname = "TOPBAR_MT_my_menu"
    bl_label = "MyMenu"
    bl_description = "拡張メニュー by " + bl_info["author"]

    # サブメニューの描画
    def draw(self, context):
        self.layout.operator("wm.url_open_preset", text="Manual", icon='HELP')

    # 既存のメニューにサブメニューを追加
    def submenu(self, context):
        self.layout.menu(TOPBAR_MT_my_menu.bl_idname)

# Blenderに登録するクラスのリスト
classes = (
    TOPBAR_MT_my_menu,
)

# アドオン有効化時コールバック
def register():
    # Blenderにクラスを登録
    for cls in classes:
        bpy.utils.register_class(cls)

    # 既存のトップバーメニューにサブメニューを追加
    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    print("レベルエディタが有効化されました。")
    
# アドオン無効化時コールバック
def unregister():
    # 既存のトップバーメニューからサブメニューを削除
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)

    # Blenderからクラスを削除
    for cls in classes:
        bpy.utils.unregister_class(cls)
    print("レベルエディタが無効化されました。")
    
# テスト実行用コード
if __name__ == "__main__":
    register()