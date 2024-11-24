//! Run Luau bytecode

// How to recompile `test.luau.bin` bytecode binary:
//
//   luau-compile --binary test.luau  > test.bc
//
// This may be required if the Luau version gets upgraded.

const std = @import("std");

// The ziglua module is made available in build.zig
const ziglua = @import("ziglua");


pub fn main() anyerror!void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer _ = gpa.deinit();

    // Initialize The Lua vm and get a reference to the main thread
    //
    // Passing a Zig allocator to the Lua state requires a stable pointer
    var lua = try ziglua.Lua.init(allocator);
    defer lua.deinit();

    // Open all Lua standard libraries
    lua.openLibs();

    // Load bytecode
    const src: [:0]const u8 = @embedFile("./test.luau");

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const State = struct {
        allocator: std.mem.Allocator,
    };

    const Context = ziglua.LuauAnalysisContext(State);

    var state = State {
        .allocator = allocator,
    };

    var context = Context.init(&state);

    context.setMode(.Strict);

    context.setReportFn(Context.wrapReportFn(struct {
        pub fn fun (_: *State, moduleName: []const u8, loc: Context.Location, ty: []const u8, msg: []const u8) void {
            std.debug.print("{s} [{s}:{}]: {s}\n", .{ty, moduleName, loc.beginLine, msg});
        }
    }.fun));

    context.setGetRequireSourceTextFn(Context.wrapGetRequireSourceTextFn(struct {
        pub fn fun (_: *State, moduleName: []const u8) ?[:0]const u8 {
            if (std.mem.eql(u8, moduleName, "test.luau")) {
                return src;
            } else {
                return null;
            }
        }
    }.fun));


    std.debug.assert(context.analyze(&[_][*:0]const u8 {"test.luau"}) == 0);
    
    const bc = try ziglua.compile(allocator, src, ziglua.CompileOptions{});
    defer allocator.free(bc);

    try lua.loadBytecode("...", bc);
    try lua.protectedCall(.{});
}
