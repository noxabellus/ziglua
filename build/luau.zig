const std = @import("std");

const Build = std.Build;
const Step = std.Build.Step;

pub fn configure(b: *Build, target: Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, upstream: *Build.Dependency, luau_use_4_vector: bool) *Step.Compile {
    const lib = b.addStaticLibrary(.{
        .name = "luau",
        .target = target,
        .optimize = optimize,
        .version = std.SemanticVersion{ .major = 0, .minor = 653, .patch = 0 },
    });

    for (luau_header_directories) |file| {
        lib.addIncludePath(upstream.path(file));
    }

    const flags = [_][]const u8{
        "-DLUA_USE_LONGJMP=1",
        "-DLUA_API=extern\"C\"",
        "-DLUACODE_API=extern\"C\"",
        "-DLUACODEGEN_API=extern\"C\"",
        if (luau_use_4_vector) "-DLUA_VECTOR_SIZE=4" else "",
    };

    lib.addCSourceFiles(.{
        .root = .{ .dependency = .{
            .dependency = upstream,
            .sub_path = "",
        } },
        .files = &luau_source_files,
        .flags = &flags,
    });
    lib.addCSourceFile(.{ .file = b.path("src/luau.cpp"), .flags = &flags });
    lib.addCSourceFile(.{ .file = b.path("src/luau_analysis.cpp"), .flags = &flags });
    lib.linkLibCpp();

    lib.installHeader(upstream.path("VM/include/lua.h"), "lua.h");
    lib.installHeader(upstream.path("VM/include/lualib.h"), "lualib.h");
    lib.installHeader(upstream.path("VM/include/luaconf.h"), "luaconf.h");
    lib.installHeader(upstream.path("Compiler/include/luacode.h"), "luacode.h");
    lib.installHeader(b.path("src/luau_analysis.h"), "luau_analysis.h");

    return lib;
}

const luau_header_directories = luau_vm_header_directories ++ luau_analysis_header_directories;
const luau_source_files = luau_vm_source_files ++ luau_analysis_source_files;

const luau_analysis_header_directories = [_][]const u8{
    "Config/include",
    "CLI",
    "EqSat/include",
    "Analysis/include",
};

const luau_analysis_source_files = [_][]const u8{
    "Config/src/Config.cpp",
    "Config/src/LinterConfig.cpp",

    "CLI/Flags.cpp",
    "CLI/FileUtils.cpp",
    "CLI/Require.cpp",

    "EqSat/src/Id.cpp",
    "EqSat/src/UnionFind.cpp",

    "Analysis/src/Anyification.cpp",
    "Analysis/src/EmbeddedBuiltinDefinitions.cpp",
    "Analysis/src/Quantify.cpp",
    "Analysis/src/Type.cpp",
    "Analysis/src/AnyTypeSummary.cpp",
    "Analysis/src/EqSatSimplification.cpp",
    "Analysis/src/Refinement.cpp",
    "Analysis/src/TypedAllocator.cpp",
    "Analysis/src/ApplyTypeFunction.cpp",
    "Analysis/src/Error.cpp",
    "Analysis/src/RequireTracer.cpp",
    "Analysis/src/TypeFunction.cpp",
    "Analysis/src/AstJsonEncoder.cpp",
    "Analysis/src/FragmentAutocomplete.cpp",
    "Analysis/src/Scope.cpp",
    "Analysis/src/TypeFunctionReductionGuesser.cpp",
    "Analysis/src/AstQuery.cpp",
    "Analysis/src/Frontend.cpp",
    "Analysis/src/Simplify.cpp",
    "Analysis/src/TypeFunctionRuntimeBuilder.cpp",
    "Analysis/src/AutocompleteCore.cpp",
    "Analysis/src/Generalization.cpp",
    "Analysis/src/Substitution.cpp",
    "Analysis/src/TypeFunctionRuntime.cpp",
    "Analysis/src/GlobalTypes.cpp",
    "Analysis/src/Subtyping.cpp",
    "Analysis/src/TypeInfer.cpp",
    "Analysis/src/Autocomplete.cpp",
    "Analysis/src/Instantiation2.cpp",
    "Analysis/src/Symbol.cpp",
    "Analysis/src/TypeOrPack.cpp",
    "Analysis/src/BuiltinDefinitions.cpp",
    "Analysis/src/Instantiation.cpp",
    "Analysis/src/TableLiteralInference.cpp",
    "Analysis/src/TypePack.cpp",
    "Analysis/src/Clone.cpp",
    "Analysis/src/IostreamHelpers.cpp",
    "Analysis/src/ToDot.cpp",
    "Analysis/src/TypePath.cpp",
    "Analysis/src/Constraint.cpp",
    "Analysis/src/JsonEmitter.cpp",
    "Analysis/src/TopoSortStatements.cpp",
    "Analysis/src/TypeUtils.cpp",
    "Analysis/src/ConstraintGenerator.cpp",
    "Analysis/src/Linter.cpp",
    "Analysis/src/ToString.cpp",
    "Analysis/src/Unifiable.cpp",
    "Analysis/src/ConstraintSolver.cpp",
    "Analysis/src/LValue.cpp",
    "Analysis/src/Transpiler.cpp",
    "Analysis/src/Unifier2.cpp",
    "Analysis/src/DataFlowGraph.cpp",
    "Analysis/src/Module.cpp",
    "Analysis/src/TxnLog.cpp",
    "Analysis/src/Unifier.cpp",
    "Analysis/src/DcrLogger.cpp",
    "Analysis/src/NonStrictTypeChecker.cpp",
    "Analysis/src/TypeArena.cpp",
    "Analysis/src/Def.cpp",
    "Analysis/src/Normalize.cpp",
    "Analysis/src/TypeAttach.cpp",
    "Analysis/src/Differ.cpp",
    "Analysis/src/OverloadResolution.cpp",
    "Analysis/src/TypeChecker2.cpp",
};

const luau_vm_header_directories = [_][]const u8{
    "Common/include",
    "Compiler/include",
    "Config/include",
    "Ast/include",
    "VM/include",
};

const luau_vm_source_files = [_][]const u8{
    "Compiler/src/BuiltinFolding.cpp",
    "Compiler/src/Builtins.cpp",
    "Compiler/src/BytecodeBuilder.cpp",
    "Compiler/src/Compiler.cpp",
    "Compiler/src/ConstantFolding.cpp",
    "Compiler/src/CostModel.cpp",
    "Compiler/src/TableShape.cpp",
    "Compiler/src/Types.cpp",
    "Compiler/src/ValueTracking.cpp",
    "Compiler/src/lcode.cpp",

    "VM/src/lapi.cpp",
    "VM/src/laux.cpp",
    "VM/src/lbaselib.cpp",
    "VM/src/lbitlib.cpp",
    "VM/src/lbuffer.cpp",
    "VM/src/lbuflib.cpp",
    "VM/src/lbuiltins.cpp",
    "VM/src/lcorolib.cpp",
    "VM/src/ldblib.cpp",
    "VM/src/ldebug.cpp",
    "VM/src/ldo.cpp",
    "VM/src/lfunc.cpp",
    "VM/src/lgc.cpp",
    "VM/src/lgcdebug.cpp",
    "VM/src/linit.cpp",
    "VM/src/lmathlib.cpp",
    "VM/src/lmem.cpp",
    "VM/src/lnumprint.cpp",
    "VM/src/lobject.cpp",
    "VM/src/loslib.cpp",
    "VM/src/lperf.cpp",
    "VM/src/lstate.cpp",
    "VM/src/lstring.cpp",
    "VM/src/lstrlib.cpp",
    "VM/src/ltable.cpp",
    "VM/src/ltablib.cpp",
    "VM/src/ltm.cpp",
    "VM/src/ludata.cpp",
    "VM/src/lutf8lib.cpp",
    "VM/src/lveclib.cpp",
    "VM/src/lvmexecute.cpp",
    "VM/src/lvmload.cpp",
    "VM/src/lvmutils.cpp",

    "Ast/src/Allocator.cpp",
    "Ast/src/Ast.cpp",
    "Ast/src/Confusables.cpp",
    "Ast/src/Lexer.cpp",
    "Ast/src/Location.cpp",
    "Ast/src/Parser.cpp",
    "Ast/src/StringUtils.cpp",
    "Ast/src/TimeTrace.cpp",
};
