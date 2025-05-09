const std = @import("std");
const builtin = @import("builtin");

const Build = std.Build;
const OptimizeMode = std.builtin.OptimizeMode;
const Compile = Build.Step.Compile;
const Module = Build.Module;

const lib_name = "webinix";
var global_log_level: std.log.Level = .warn;

/// Vendored dependencies of webinix.
pub const Dependency = enum {
    civetweb,
    // TODO: Check and add all vendored dependencies, e.g. "webview"
};

const DebugDependencies = std.EnumSet(Dependency);

pub fn build(b: *Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const is_dynamic = b.option(bool, "dynamic", "build the dynamic library") orelse false;
    const enable_tls = b.option(bool, "enable-tls", "enable TLS support") orelse false;
    const verbose = b.option(std.log.Level, "verbose", "set verbose output") orelse .warn;
    global_log_level = verbose;
    // TODO: Support list of dependencies once support is limited to >0.13.0
    const debug = b.option(Dependency, "debug", "enable dependency debug output");
    const debug_dependencies = DebugDependencies.initMany(if (debug) |d| &.{d} else &.{});

    if (enable_tls and !target.query.isNative()) {
        log(.err, .Webinix, "cross compilation is not supported with TLS enabled", .{});
        return error.InvalidBuildConfiguration;
    }

    log(.info, .Webinix, "Building {s} Webinix library{s}...", .{
        if (is_dynamic) "dynamic" else "static",
        if (enable_tls) " with TLS support" else "",
    });
    defer {
        log(.info, .Webinix, "Done.", .{});
    }

    const webinix = if (is_dynamic) b.addSharedLibrary(.{
        .name = lib_name,
        .target = target,
        .optimize = optimize,
        .pic = true,
    }) else b.addStaticLibrary(.{
        .name = lib_name,
        .target = target,
        .optimize = optimize,
    });
    try addLinkerFlags(b, webinix, enable_tls, debug_dependencies);

    b.installArtifact(webinix);

    try build_examples(b, webinix);
}

fn addLinkerFlags(
    b: *Build,
    webinix: *Compile,
    enable_tls: bool,
    debug_dependencies: DebugDependencies,
) !void {
    const webinix_target = webinix.rootModuleTarget();
    const is_windows = webinix_target.os.tag == .windows;
    const is_darwin = webinix_target.os.tag == .macos;
    const debug = webinix.root_module.optimize.? == .Debug;

    // Prepare compiler flags.
    const no_tls_flags: []const []const u8 = &.{"-DNO_SSL"};
    const tls_flags: []const []const u8 = &.{ "-DWEBUI_TLS", "-DNO_SSL_DL", "-DOPENSSL_API_1_1" };
    const civetweb_flags: []const []const u8 = &.{
        "-DNO_CACHING",
        "-DNO_CGI",
        "-DUSE_WEBSOCKET",
        "-Wno-error=date-time",
    };

    if (debug) {
        webinix.root_module.addCMacro("WEBUI_LOG", "");
    }
    webinix.addCSourceFile(.{
        .file = b.path("src/webinix.c"),
        .flags = if (enable_tls) tls_flags else no_tls_flags,
    });

    const civetweb_debug = debug and debug_dependencies.contains(.civetweb);
    webinix.addCSourceFile(.{
        .file = b.path("src/civetweb/civetweb.c"),
        .flags = if (enable_tls and !civetweb_debug)
            civetweb_flags ++ tls_flags ++ .{"-DNDEBUG"}
        else if (enable_tls and civetweb_debug)
            civetweb_flags ++ tls_flags
        else if (!enable_tls and !civetweb_debug)
            civetweb_flags ++ .{"-DUSE_WEBSOCKET"} ++ no_tls_flags ++ .{"-DNDEBUG"}
        else
            civetweb_flags ++ .{"-DUSE_WEBSOCKET"} ++ no_tls_flags,
    });
    webinix.linkLibC();
    webinix.addIncludePath(b.path("include"));
    webinix.installHeader(b.path("include/webinix.h"), "webinix.h");
    if (is_darwin) {
        webinix.addCSourceFile(.{
            .file = b.path("src/webview/wkwebview.m"),
            .flags = &.{},
        });
        webinix.linkFramework("Cocoa");
        webinix.linkFramework("WebKit");
    } else if (is_windows) {
        webinix.linkSystemLibrary("ws2_32");
        webinix.linkSystemLibrary("ole32");
        if (webinix_target.abi == .msvc) {
            webinix.linkSystemLibrary("Advapi32");
            webinix.linkSystemLibrary("Shell32");
            webinix.linkSystemLibrary("user32");
        }
        if (enable_tls) {
            webinix.linkSystemLibrary("bcrypt");
        }
    }
    if (enable_tls) {
        webinix.linkSystemLibrary("ssl");
        webinix.linkSystemLibrary("crypto");
    }

    for (webinix.root_module.link_objects.items) |lo| {
        switch (lo) {
            .c_source_file => |csf| {
                log(.debug, .Webinix, "{s} linker flags: {s}", .{
                    csf.file.src_path.sub_path,
                    csf.flags,
                });
            },
            else => {},
        }
    }
}

fn build_examples(b: *Build, webinix: *Compile) !void {
    const build_examples_step = b.step("examples", "builds the library and its examples");
    const target = webinix.root_module.resolved_target.?;
    const optimize = webinix.root_module.optimize.?;

    const examples_path = b.path("examples/C").getPath(b);
    var examples_dir = std.fs.cwd().openDir(
        examples_path,
        .{ .iterate = true },
    ) catch |e| switch (e) {
        // Do not attempt building examples if directory does not exist.
        error.FileNotFound => return,
        else => return e,
    };
    defer examples_dir.close();

    var paths = examples_dir.iterate();
    while (try paths.next()) |val| {
        if (val.kind != .directory) {
            continue;
        }
        const example_name = val.name;

        const exe = b.addExecutable(.{
            .name = example_name,
            .target = target,
            .optimize = optimize,
        });
        const path = try std.fmt.allocPrint(b.allocator, "examples/C/{s}/main.c", .{example_name});
        defer b.allocator.free(path);

        exe.addCSourceFile(.{ .file = b.path(path), .flags = &.{} });
        exe.linkLibrary(webinix);

        const exe_install = b.addInstallArtifact(exe, .{});
        const exe_run = b.addRunArtifact(exe);
        const step_name = try std.fmt.allocPrint(b.allocator, "run_{s}", .{example_name});
        defer b.allocator.free(step_name);
        const step_desc = try std.fmt.allocPrint(b.allocator, "run example {s}", .{example_name});
        defer b.allocator.free(step_desc);

        const cwd = try std.fmt.allocPrint(b.allocator, "src/examples/{s}", .{example_name});
        defer b.allocator.free(cwd);
        exe_run.setCwd(b.path(cwd));

        exe_run.step.dependOn(&exe_install.step);
        build_examples_step.dependOn(&exe_install.step);
        b.step(step_name, step_desc).dependOn(&exe_run.step);
    }
}

/// Function to runtime-scope log levels based on build flag, for all scopes.
fn log(
    comptime level: std.log.Level,
    comptime scope: @TypeOf(.EnumLiteral),
    comptime format: []const u8,
    args: anytype,
) void {
    const should_print: bool = @intFromEnum(global_log_level) >= @intFromEnum(level);
    if (should_print) {
        switch (comptime level) {
            .err => std.log.scoped(scope).err(format, args),
            .warn => std.log.scoped(scope).warn(format, args),
            .info => std.log.scoped(scope).info(format, args),
            .debug => std.log.scoped(scope).debug(format, args),
        }
    }
}
