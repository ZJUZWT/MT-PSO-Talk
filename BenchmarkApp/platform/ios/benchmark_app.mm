// Minimal iOS compression benchmark app.
// Runs compression and decompression benchmarks on launch and displays results.

#import <UIKit/UIKit.h>
#import <Metal/Metal.h>

#include "orchestrator/benchmark_orchestrator.h"
#include <string>
#include <fstream>

@interface BenchmarkViewController : UIViewController
@property (nonatomic, strong) UITextView* textView;
@property (nonatomic, strong) UIButton* runButton;
@end

@implementation BenchmarkViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor systemBackgroundColor];

    self.runButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [self.runButton setTitle:@"Run Compression Benchmark" forState:UIControlStateNormal];
    self.runButton.titleLabel.font = [UIFont boldSystemFontOfSize:20];
    self.runButton.frame = CGRectMake(20, 60, self.view.bounds.size.width - 40, 50);
    [self.runButton addTarget:self action:@selector(runBenchmark) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:self.runButton];

    self.textView = [[UITextView alloc] initWithFrame:CGRectMake(20, 120,
        self.view.bounds.size.width - 40, self.view.bounds.size.height - 140)];
    self.textView.font = [UIFont fontWithName:@"Menlo" size:10];
    self.textView.editable = NO;
    self.textView.text = @"Tap 'Run' to start compression and decompression benchmarks...";
    [self.view addSubview:self.textView];
}

- (void)runBenchmark {
    self.textView.text = @"Running benchmark...\n";
    [self.runButton setEnabled:NO];

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
        benchmark::OrchestratorConfig config;
        config.enable_graphics = false;
        config.graphics_iterations = 3;
        config.compression_iterations = 5;

        benchmark::BenchmarkOrchestrator orchestrator;
        auto report = orchestrator.run(config);

        std::string output;
        output += report.compression_matrix_text + "\n\n";
        output += report.summary_text + "\n";

        // Save JSON to Documents
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* docDir = [paths firstObject];
        NSString* jsonPath = [docDir stringByAppendingPathComponent:@"compression_results.json"];
        NSString* csvPath = [docDir stringByAppendingPathComponent:@"compression_results.csv"];
        std::ofstream f([jsonPath UTF8String]);
        f << report.full_json;
        f.close();

        std::ofstream csvFile([csvPath UTF8String]);
        csvFile << report.compression_csv;
        csvFile.close();

        output += "\nJSON saved to: " + std::string([jsonPath UTF8String]) + "\n";
        output += "CSV saved to: " + std::string([csvPath UTF8String]) + "\n";

        NSString* resultText = [NSString stringWithUTF8String:output.c_str()];

        dispatch_async(dispatch_get_main_queue(), ^{
            self.textView.text = resultText;
            [self.runButton setEnabled:YES];
        });
    });
}

@end

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow* window;
@end

@implementation AppDelegate
- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)opts {
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.rootViewController = [[BenchmarkViewController alloc] init];
    [self.window makeKeyAndVisible];
    return YES;
}
@end

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
