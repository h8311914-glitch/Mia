/**
 * Mia's hello world.
 * Simply shows a rotating mia icon and logs "hello world" once at startup
 */


/**
 * "mia.h" will include (most) of the Mia engine.
 * Which will result in the modules:
 *
 * - "o" Object oriented standard library
 * - "m" Math and linear algebra
 * - "s" Sound stuff
 * - "r" Render stuff working on OpenGL(ES|WEB)
 * - "a" App Scenes, Views, and user input
 * - "u" Utilities
 * - "w" Widgets: GUI windows, buttons, input fields
 * - "x" Extended stuff like a virtual keyboard or color picker widget
 *
 * Each module has a dependency to the upper one.
 */
#include "mia.h"



/**
 * Naming Conventions:
 * 
 * - **Types and Objects:** 
 *   - oobj: simple typedef to void* (primitive types in o/common.h).
 *   - OObj, OList, RTex, AView: PascalCase for object names.
 *   
 * - **Functions and Variables:**
 *   - o_clear, o_str_equals: snake_case for functions and variables.
 *   
 * - **Object-Specific Functions:**
 *   - OObj_name, RTex_blit: ObjectName_snake_case.
 *   
 * - **Prefixes:**
 *   - Functions and objects have a module prefix: "o_"log, "O"Obj, "R"Tex, "A"View.
 *   
 * - **Header Files:**
 *   - Named by their object or specific group of functions.
 *   - Functions in these headers are usually prefixed: o/str.h -> o_str_clone, o_str_find.
 *   
 */


/**
 * All stuff used in this example put in a static struct.
 */
static struct {

    /**
     * oobj is a simple typedef to void* ( o/common.h ).
     * Mia uses this to pass OObj's and it's subclasses (mia object; o/OObj.h )
     * In this case, sprite will be of type RTex* ( r/RTex.h )
     *      which is an object to handle GPU textures of the "r"ender module
     */
    oobj sprite;

    /**
     * angle position in [0:1]
     */
    float sprite_t;
} L;


/**
 * This function is called once, when this example is started
 * @param view AView object of this example scene ( a/AView.h )
 *             More infos about AView is at the end of this example
 */
static void setup(oobj view)
{
    /**
     * logs an info string ( o/log.h )
     * also accepts formatted strings like: o_log("value[%i]=%f", i, value[i]);
     */
    o_log("hello world");


    /**
     * memset to 0 ( o/common.h )
     * the common header functions and stuff only use "o_" as prefix, not "o_common_clear" as others would do.
     * Because the classic memset is in the group of function that expects a byte size,
     *      Mia wraps it into (element_size, count) to avoid (common) bugs
     * Others are o_memcpy and o_memmove which also have an element_size as parameter
     */
    o_clear(&L, sizeof L, 1);



    /**
     * Mia's object management system automatically handles the deletion of resources
     * and objects through a hierarchical parent-child relationship.
     * When an object (e.g. view) is deleted, all associated resources (textures, sounds, etc.)
     * and child objects are recursively deleted. This significantly reduces the need for manual
     * memory management (e.g. free calls), making resource management more intuitive and less error-prone.
     */

    /**
     * An RTex is an object which holds and works with texture data on the GPU ( r/RTex.h )
     * "*_new" are constructors,
     * "*_new_file" is a specialized constructor, in this case it loads the contents of the given image file.
     *
     * view (AView object of this setup function) will get the parent of the RTex
     *
     * Mia uses special routes to different file locations to make things easier.
     *   - "$..." uses the "res/" resources directory for static assets
     *   - "#..." are temporary files (may or may not be available on next app start)
     *   - "&..." for saving files, which should be available in the next app start
     *   - "..." Native file path, use with caution and will be warned by default
     *           To be platform independent, most stuff should use the first three cases `$ # &`
     */
    L.sprite = RTex_new_file(view, "$ex/icon16.png");
}

/**
 * This function is called every frame
 * @param view AView object of this example scene
 * @param tex RTex object to render to (should be done in the render function)
 * @param dt delta time; time between this and the last frame
 */
static void update(oobj view, oobj tex, float dt)
{
    /**
     * Place your simulation loop here.
     * Also (pointer) events should be handled here and not in the render function
     */
}

/**
 * This function is called every frame to actually render stuff
 * @param view AView object of this example scene
 * @param tex RTex object to render to
 * @param dt delta time; time between this and the last frame (in seconds)
 */
static void render(oobj view, oobj tex, float dt)
{
    /**
     * A full rotation in 8 seconds
     */
    L.sprite_t += dt / 8.0f;

    /**
     * float modulo from 0.0 -> 1.0, m_fract could have also been used ( m/sca/flt.h; sca:=scalar )
     */
    L.sprite_t = m_mod(L.sprite_t, 1.0f);

    float angle = L.sprite_t * 2 * m_PI;
    
    /**
     * x and y as left top sprite position.
     * The coordination system is (as most 2d engines...):
     *     Origin is at the left top screen border
     *     +x -> to the right
     *     +y -> to the bottom
     *
     *     O----->  X-AXIS
     *     |
     *     |
     *     V
     *
     *     Y       [][][][]
     *     '       [SPRITE]
     *     A       [][][][]
     *     X
     *     I
     *     S
     */
    float x = 82 + 82 * m_cos(angle);
    float y = 82 + 82 * m_sin(angle);

    /**
     * blits (not blends) the icon RTex (loaded in setup) onto the RTex "tex" from the AView.
     * Blitting replaces old image pixels with the new one, ignoring alpha blending
     * ( r/RTex_ex.h )
     */
    RTex_blit(tex, L.sprite, x, y);

    /**
     * Will show a rotating mia icon.
     * The screen is cleared to black on each frame at the frame start.
     * The camera uses a size of at least 180x180 units 
     *      and uses an integer scaling to real pixels (pixel perfect; as default).
     * The size of 180 units is an anti prime and the smaller size of common screen sizes are a multiple of it.
     *      Like 1080=6*180 (6 pixel form a unit) or 1440=8*180; 720=4*180; ...
     * So on some devices the logo will also touch the right or bottom border
     *      (82+82+16 == 180)
     */
}



/**
 * Starting point of this example,
 * called from src/ex/main.c which itself is called by src/main.c
 *
 * > If you want to create your own stuff, edit apps/hello/main.c ;) <
 *
 * @param root OObj to allocate on, if this gets deleted, the full example will get deleted (in recursion)
 *             Exactly this happens if you click on the "X" button on the upper left.
 *                  Which is a transparent scene rendered on top of the example scenes.
 */
oobj ex_00_main(oobj root)
{
    /**
     * Creates an AView object with the functions above,
     * allocated on the given root object ( a/AView.h )
     */
    oobj view = AView_new(root, setup, update, render);

    /**
     * The actual AScene which will display the AView on the screen ( a/AScene.h )
     * AScene's work together with the app's scenes stack to render the AView's.
     * An AView itself will not be rendered until its functions are executed elsewhere.
     * So each AScene needs an AView, but AView's could be used elsewhere (like in a USplit ( u/USplit.h ) )
     * the last parameter of the AScene_new constructor is a bool flag if the view should be moved into the AScene.
     * Because each OObj has a single parent, its sometimes necessary to move objects to change the parent.
     * if true, the AView "view" will be o_move'd into the new AScene like: o_move(view, scene).
     * Last parameter is controlling the viewport. Its either:
     *   AScene_SAFE:    Uses the safe zone in which every pixel in the AView is safe to use.
     *   AScene_UNSAFE:  On devices like modern smartphones,
     *                        the unsafe zone includes areas which should not be used for interactivity.
     *                        Like the region of the status bar, which may be behind camera holes and nodges.
     *                        Or at the bottom where the floating android home buttons are located.
     *                        Have a look at AScene_safe_padding() to pad gui elements into the safe zone.
     *   AScene_CUSTOM:  Lets you create a custom viewport with AScene_viewport_set
     */
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}


/**
 * Summary:
 *
 * In this example, we've explored the basics of creating a simple scene with Mia,
 * demonstrating object initialization, resource management, and rendering.
 * Understanding these core concepts is essential as you delve deeper into the engine.
 * I hope you got already familiar with the base concepts of Mia.
 * Which are:
 * - An object system which also manages resources in C.
 * - An easy to follow set of naming rules, which differ from other standards,
 *      but makes navigating and tracing objects more charming, also nice for auto completion.
 *      Feel free to surf through the header files of the different modules.
 *      Each header file acts as classic documentation.
 * - Platform independent app development with a fast compile/link time in mind.
 *      Just try it! Clone the repo, have a look at "doc/install.md" and compile the whole engine in a few seconds.
 *      Working with mia feels like working in a scripting language, a small change can be tested moments later!
 *
 * To get a little more familiar with Mia, consider looking at some header files, which serve as api documentation.
 * The following headers are from the base module 'o', located in "<mia_repo>/mia/include/o/..."
 *
 * - "o/common.h"       Some basic macros and functions like typedefs for osize, or safer versions of memcpy, etc.
 * - "o/log.h"          Logging functions
 * - "o/OObj.h"         Base Object and its system from which all oobj's inherit
 * - "o/OArray.h"       Dynamic array with multiple options for faster resizing. Mia's version of cxx's std::vector
 * - "o/OList.h"        Uses an OArray internally but value type is always void*, perfect for a list of strings or oobjs
 * - "o/OMap.h"         Hashmap with custom key type and value type
 * - "o/ODict.h"        Uses an OMap internally with string keys and void* values, analog to OList
 * - "o/file.h"         Functions to read and write files or open em as OStream's
 * - "o/str.h"          Functions for string manipulation
 * - "o/Json.h"         Json parser and writer, Mia's json lib makes use of the OObj tree
 */
