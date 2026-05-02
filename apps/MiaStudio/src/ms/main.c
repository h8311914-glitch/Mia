#include "mp/selection.h"
#ifdef MIA_OPTION_APP_MS_MAIN
#define MIA_MAIN
#include "o/common.h"
#endif

#include "ms/ms.h"
#include "mia.h"

struct main_context {
    int i;
};


static void setup(oobj view)
{
    struct main_context *C = o_user(view);


}

static void update(oobj view, oobj tex, float dt)
{
    struct main_context *C = o_user(view);

}

static void render(oobj view, oobj tex, float dt)
{
    struct main_context *C = o_user(view);

    RTex_clear(tex, R_CYAN);
}


//
// public
//


oobj ms_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);

    struct main_context *C = o_user_new0(view, *C, 1);

    return scene;
}

oobj ms_main_splashed(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj splash = u_splash_new_mia(root, view, true, AScene_SAFE,
                                   "HORSIMANN", "MIA STUDIO", 2.0f, true);

    struct main_context *C = o_user_new0(view, *C, 1);

    return splash;

}

#ifdef MIA_OPTION_APP_MS_MAIN
static void app_main(oobj root)
{
    x_install();
    ms_main_splashed(root);
}

int main(int argc, char **argv)
{
    struct a_app_run_options options = a_app_run_options_default();
    options.log_level = O_LOG_DEBUG;
    a_app_run(app_main, &options);
    return 0;
}
#endif
