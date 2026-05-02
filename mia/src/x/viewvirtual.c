#include "x/viewvirtual.h"
#include "x/XViewVirtual.h"
#include "a/AScene.h"
#include "a/AViewStage.h"
#include "a/app.h"

oobj x_viewvirtual_scene(oobj parent, AView *view, bool move_view, ivec2 resolution)
{
    XViewVirtual *virt = XViewVirtual_new(parent, view, move_view, resolution);
    AScene *scene = AScene_new(parent, virt, true, AScene_SAFE);
    AScene_opaque_set(scene, true);
    return scene;
}

oobj x_viewvirtual_scene_install_stage(oobj parent, ivec2 resolution)
{
    oobj stage = AViewStage_new(parent);
    oobj scene = x_viewvirtual_scene(parent, stage, true, resolution);
    a_app_stage_set(stage);
    return scene;
}
