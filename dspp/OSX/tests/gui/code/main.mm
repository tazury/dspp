//#include "window.h"
#include <Logger.h>
//
//
//int main() {

//
//	Window window(1920, 720, "DSPP Testing");
//	window.Run();
//}


#import <Foundation/Foundation.h>
#import <iostream>
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_metal.h"
#include "imgui/imgui_impl_osx.h"
#include "UIEngine.h"
#include "elements/testElement.h"

@interface AppViewController : NSViewController<NSWindowDelegate>
@end


@interface AppViewController () <MTKViewDelegate>
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;
@end

UIEngine* engine = nullptr;

//-----------------------------------------------------------------------------------
// AppViewController
//-----------------------------------------------------------------------------------

@implementation AppViewController

-(instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];

    if (!self.device)
    {
        NSLog(@"Metal is not supported");
        abort();
    }

    DSLogger::addListener([](const LogEntry& entry) {
        const char* typeStr = "Unknown";
        switch (entry.type) {
        case DSLogTypes::Normal: typeStr = "Normal"; break;
        case DSLogTypes::Warning: typeStr = "Warning"; break;
        case DSLogTypes::Error: typeStr = "Error"; break;
        }

        std::cout << "[" << typeStr << "] "
            << entry.file << ":" << entry.line << " - "
            << entry.message << "\n";
        });

    engine = new UIEngine(_device, _commandQueue);

    auto winElementUI = std::make_shared<WindowElement>(
        "winElement"
    );

    engine->AddElement(winElementUI);


    return self;
}

-(MTKView *)mtkView
{
    return (MTKView *)self.view;
}

-(void)loadView
{
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, 1200, 720)];
}

-(void)viewDidLoad
{
    [super viewDidLoad];

    self.mtkView.device = self.device;
    self.mtkView.delegate = self;

#if TARGET_OS_OSX
    ImGui_ImplOSX_Init(self.view);
    [NSApp activateIgnoringOtherApps:YES];
#endif
}

-(void)drawInMTKView:(MTKView*)view
{
 engine->Render(view);
}

-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    self.view.window.delegate = self;

}

- (void)windowWillClose:(NSNotification *)notification
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();
    ImGui::DestroyContext();
    exit(0);
}
@end


int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // Create the application instance
        [NSApplication sharedApplication];

        // Create the window
        NSRect frame = NSMakeRect(0, 0, 1200, 720);
        NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                       styleMask:(NSWindowStyleMaskTitled |
                                                                  NSWindowStyleMaskClosable |
                                                                  NSWindowStyleMaskResizable)
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];

        [window setTitle:@"DSPP Testing"];

        // Set your custom view controller
        AppViewController *vc = [[AppViewController alloc] init];
        [window setContentViewController:vc];

        // Show the window
        [window makeKeyAndOrderFront:nil];

        // Start the app event loop
        [NSApp run];
    }
    return 0;
}