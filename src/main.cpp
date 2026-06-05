#define SOKOL_IMPL
#define SOKOL_GLES3
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"

static sg_pass_action pass_action;

static void init(void) {
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].clear_value = { 0.1f, 0.1f, 0.2f, 1.0f };
}

static void frame(void) {
    sg_begin_pass({ .action = pass_action, .swapchain = sglue_swapchain() });
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.width = 800;
    desc.height = 600;
    desc.window_title = "EmberGUI";
    desc.logger.func = slog_func;
    return desc;
}
