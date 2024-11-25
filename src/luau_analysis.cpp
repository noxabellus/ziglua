// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#include "Luau/Config.h"
#include "Luau/ModuleResolver.h"
#include "Luau/TypeInfer.h"
#include "Luau/BuiltinDefinitions.h"
#include "Luau/Frontend.h"
#include "Luau/TypeAttach.h"
#include "Luau/Transpiler.h"
#include "Luau/ExperimentalFlags.h"

extern "C" {
    #include "luau_analysis.h"
};

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <fstream>

LUAU_FASTFLAG(DebugLuauTimeTracing)
LUAU_FASTFLAG(DebugLuauLogSolverToJsonFile)

static void applyConfig(ZigLuauConfig& conf, Luau::Config& nativeConfig)
{
    nativeConfig.mode = conf.mode == ZLA_STRICT ? Luau::Mode::Strict : (conf.mode == ZLA_DEFINITION? Luau::Mode::Definition : Luau::Mode::Nonstrict);
    nativeConfig.parseOptions.allowDeclarationSyntax = conf.allowDeclarationSyntax;
    nativeConfig.parseOptions.captureComments = conf.captureComments;

    if (conf.enabledLint.Unknown) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_Unknown);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_Unknown);
    if (conf.enabledLint.UnknownGlobal) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_UnknownGlobal);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_UnknownGlobal);
    if (conf.enabledLint.DeprecatedGlobal) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_DeprecatedGlobal);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_DeprecatedGlobal);
    if (conf.enabledLint.GlobalUsedAsLocal) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_GlobalUsedAsLocal);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_GlobalUsedAsLocal);
    if (conf.enabledLint.LocalShadow) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_LocalShadow);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_LocalShadow);
    if (conf.enabledLint.SameLineStatement) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_SameLineStatement);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_SameLineStatement);
    if (conf.enabledLint.MultiLineStatement) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_MultiLineStatement);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_MultiLineStatement);
    if (conf.enabledLint.LocalUnused) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_LocalUnused);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_LocalUnused);
    if (conf.enabledLint.FunctionUnused) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_FunctionUnused);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_FunctionUnused);
    if (conf.enabledLint.ImportUnused) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_ImportUnused);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_ImportUnused);
    if (conf.enabledLint.BuiltinGlobalWrite) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_BuiltinGlobalWrite);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_BuiltinGlobalWrite);
    if (conf.enabledLint.PlaceholderRead) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_PlaceholderRead);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_PlaceholderRead);
    if (conf.enabledLint.UnreachableCode) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_UnreachableCode);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_UnreachableCode);
    if (conf.enabledLint.UnknownType) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_UnknownType);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_UnknownType);
    if (conf.enabledLint.ForRange) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_ForRange);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_ForRange);
    if (conf.enabledLint.UnbalancedAssignment) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_UnbalancedAssignment);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_UnbalancedAssignment);
    if (conf.enabledLint.ImplicitReturn) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_ImplicitReturn);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_ImplicitReturn);
    if (conf.enabledLint.DuplicateLocal) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_DuplicateLocal);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_DuplicateLocal);
    if (conf.enabledLint.FormatString) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_FormatString);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_FormatString);
    if (conf.enabledLint.TableLiteral) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_TableLiteral);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_TableLiteral);
    if (conf.enabledLint.UninitializedLocal) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_UninitializedLocal);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_UninitializedLocal);
    if (conf.enabledLint.DuplicateFunction) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_DuplicateFunction);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_DuplicateFunction);
    if (conf.enabledLint.DeprecatedApi) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_DeprecatedApi);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_DeprecatedApi);
    if (conf.enabledLint.TableOperations) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_TableOperations);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_TableOperations);
    if (conf.enabledLint.DuplicateCondition) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_DuplicateCondition);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_DuplicateCondition);
    if (conf.enabledLint.MisleadingAndOr) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_MisleadingAndOr);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_MisleadingAndOr);
    if (conf.enabledLint.CommentDirective) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_CommentDirective);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_CommentDirective);
    if (conf.enabledLint.IntegerParsing) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_IntegerParsing);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_IntegerParsing);
    if (conf.enabledLint.ComparisonPrecedence) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_ComparisonPrecedence);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_ComparisonPrecedence);
    if (conf.enabledLint.RedundantNativeAttribute) nativeConfig.enabledLint.enableWarning(Luau::LintWarning::Code_RedundantNativeAttribute);
    else nativeConfig.enabledLint.disableWarning(Luau::LintWarning::Code_RedundantNativeAttribute);

    if (conf.fatalLint.Unknown) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_Unknown);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_Unknown);
    if (conf.fatalLint.UnknownGlobal) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_UnknownGlobal);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_UnknownGlobal);
    if (conf.fatalLint.DeprecatedGlobal) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_DeprecatedGlobal);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_DeprecatedGlobal);
    if (conf.fatalLint.GlobalUsedAsLocal) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_GlobalUsedAsLocal);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_GlobalUsedAsLocal);
    if (conf.fatalLint.LocalShadow) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_LocalShadow);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_LocalShadow);
    if (conf.fatalLint.SameLineStatement) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_SameLineStatement);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_SameLineStatement);
    if (conf.fatalLint.MultiLineStatement) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_MultiLineStatement);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_MultiLineStatement);
    if (conf.fatalLint.LocalUnused) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_LocalUnused);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_LocalUnused);
    if (conf.fatalLint.FunctionUnused) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_FunctionUnused);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_FunctionUnused);
    if (conf.fatalLint.ImportUnused) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_ImportUnused);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_ImportUnused);
    if (conf.fatalLint.BuiltinGlobalWrite) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_BuiltinGlobalWrite);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_BuiltinGlobalWrite);
    if (conf.fatalLint.PlaceholderRead) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_PlaceholderRead);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_PlaceholderRead);
    if (conf.fatalLint.UnreachableCode) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_UnreachableCode);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_UnreachableCode);
    if (conf.fatalLint.UnknownType) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_UnknownType);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_UnknownType);
    if (conf.fatalLint.ForRange) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_ForRange);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_ForRange);
    if (conf.fatalLint.UnbalancedAssignment) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_UnbalancedAssignment);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_UnbalancedAssignment);
    if (conf.fatalLint.ImplicitReturn) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_ImplicitReturn);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_ImplicitReturn);
    if (conf.fatalLint.DuplicateLocal) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_DuplicateLocal);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_DuplicateLocal);
    if (conf.fatalLint.FormatString) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_FormatString);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_FormatString);
    if (conf.fatalLint.TableLiteral) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_TableLiteral);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_TableLiteral);
    if (conf.fatalLint.UninitializedLocal) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_UninitializedLocal);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_UninitializedLocal);
    if (conf.fatalLint.DuplicateFunction) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_DuplicateFunction);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_DuplicateFunction);
    if (conf.fatalLint.DeprecatedApi) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_DeprecatedApi);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_DeprecatedApi);
    if (conf.fatalLint.TableOperations) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_TableOperations);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_TableOperations);
    if (conf.fatalLint.DuplicateCondition) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_DuplicateCondition);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_DuplicateCondition);
    if (conf.fatalLint.MisleadingAndOr) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_MisleadingAndOr);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_MisleadingAndOr);
    if (conf.fatalLint.CommentDirective) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_CommentDirective);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_CommentDirective);
    if (conf.fatalLint.IntegerParsing) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_IntegerParsing);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_IntegerParsing);
    if (conf.fatalLint.ComparisonPrecedence) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_ComparisonPrecedence);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_ComparisonPrecedence);
    if (conf.fatalLint.RedundantNativeAttribute) nativeConfig.fatalLint.enableWarning(Luau::LintWarning::Code_RedundantNativeAttribute);
    else nativeConfig.fatalLint.disableWarning(Luau::LintWarning::Code_RedundantNativeAttribute);

    nativeConfig.lintErrors = conf.lintErrors;
    nativeConfig.typeErrors = conf.typeErrors;

    nativeConfig.globals.clear();
    for (int i = 0; i < conf.numGlobals; i++) {
        nativeConfig.globals.push_back(conf.globals[i]);
    }
}

struct ConfigResolver : Luau::ConfigResolver
{
    ZigLuauAnalysisContext* zla_ctx;
    mutable Luau::Config nativeConfig;

    ConfigResolver(ZigLuauAnalysisContext* x)
    : zla_ctx(x)
    { }

    const Luau::Config& getConfig(const Luau::ModuleName& name) const override
    {
        ZigLuauConfig config = zla_ctx->config;

        if (zla_ctx->configResolverFn != nullptr) {
            zla_ctx->configResolverFn(zla_ctx->userstate, &config);
        }

        applyConfig(config, nativeConfig);

        return nativeConfig;
    }
};

struct FileResolver : Luau::FileResolver
{
    ZigLuauAnalysisContext* zla_ctx;

    FileResolver (ZigLuauAnalysisContext* x)
    : zla_ctx(x)
    { }

    std::optional<Luau::SourceCode> readSource(const Luau::ModuleName& name) override
    {
        if (zla_ctx->getRequireSourceTextFn == nullptr)
            return std::nullopt;

        Luau::SourceCode::Type sourceType;
        std::optional<std::string> source = std::nullopt;

        char const* sourcePtr = zla_ctx->getRequireSourceTextFn(zla_ctx->userstate, name.c_str());

        if (sourcePtr == nullptr)
            return std::nullopt;

        return Luau::SourceCode{sourcePtr, Luau::SourceCode::Module};
    }

    std::optional<Luau::ModuleInfo> resolveModule(const Luau::ModuleInfo* context, Luau::AstExpr* node) override
    {
        if (Luau::AstExprConstantString* expr = node->as<Luau::AstExprConstantString>())
        {
            std::string path{expr->value.data, expr->value.size};

            return {{path}};
        }

        return std::nullopt;
    }

    std::string getHumanReadableModuleName(const Luau::ModuleName& name) const override
    {
        if (name == "-")
            return "stdin";
        return name;
    }
};


static void report(ZigLuauAnalysisContext* zla_ctx, char const* name, const Luau::Location& loc, char const* type, char const* message)
{
    if (zla_ctx->reportFn != nullptr) zla_ctx->reportFn(zla_ctx->userstate, name, ZigLuauLocation { loc.begin.line, loc.begin.column, loc.end.line, loc.end.column }, type, message);
}

static void reportError(ZigLuauAnalysisContext* zla_ctx, const Luau::TypeError& error)
{
    auto frontend = (Luau::Frontend*) zla_ctx->frontend;
    std::string humanReadableName = frontend->fileResolver->getHumanReadableModuleName(error.moduleName);

    if (const Luau::SyntaxError* syntaxError = Luau::get_if<Luau::SyntaxError>(&error.data))
        report(zla_ctx, humanReadableName.c_str(), error.location, "SyntaxError", syntaxError->message.c_str());
    else
        report(
            zla_ctx,
            humanReadableName.c_str(),
            error.location,
            "TypeError",
            Luau::toString(error, Luau::TypeErrorToStringOptions{frontend->fileResolver}).c_str()
        );
}

static void reportWarning(ZigLuauAnalysisContext* zla_ctx, const char* name, const Luau::LintWarning& warning)
{
    report(zla_ctx, name, warning.location, Luau::LintWarning::getName(warning.code), warning.text.c_str());
}

static bool reportModuleResult(ZigLuauAnalysisContext* zla_ctx, const Luau::ModuleName& name)
{
    auto frontend = (Luau::Frontend*) zla_ctx->frontend;
    std::optional<Luau::CheckResult> cr = frontend->getCheckResult(name, false);

    if (!cr)
    {
        report(zla_ctx, name.c_str(), Luau::Location{}, "Internal", "tried to report a module result for a non-existent module");
        return false;
    }

    if (!frontend->getSourceModule(name))
    {
        report(zla_ctx, name.c_str(), Luau::Location{}, "Not Found", "could not find this module");
        return false;
    }

    for (auto& error : cr->errors)
        reportError(zla_ctx, error);

    std::string humanReadableName = frontend->fileResolver->getHumanReadableModuleName(name);
    for (auto& error : cr->lintResult.errors)
        reportWarning(zla_ctx, humanReadableName.c_str(), error);
    for (auto& warning : cr->lintResult.warnings)
        reportWarning(zla_ctx, humanReadableName.c_str(), warning);

    bool okay = cr->errors.empty() && cr->lintResult.errors.empty();

    if (okay && zla_ctx->annotateFn != nullptr)
    {
        Luau::SourceModule* sm = frontend->getSourceModule(name);
        Luau::ModulePtr m = frontend->moduleResolver.getModule(name);

        Luau::attachTypeData(*sm, *m);

        std::string annotated = Luau::transpileWithTypes(*sm->root);

        zla_ctx->annotateFn(zla_ctx->userstate, name.c_str(), annotated.c_str());
    }

    return okay;
}


static int assertionHandler(const char* expr, const char* file, int line, const char* function)
{
    printf("%s(%d): LUAU INTERNAL ASSERTION FAILED: %s\n", file, line, expr);
    fflush(stdout);
    return 1;
}







struct TaskScheduler
{
    TaskScheduler(unsigned threadCount)
        : threadCount(threadCount)
    {
        for (unsigned i = 0; i < threadCount; i++)
        {
            workers.emplace_back(
                [this]
                {
                    workerFunction();
                }
            );
        }
    }

    ~TaskScheduler()
    {
        for (unsigned i = 0; i < threadCount; i++)
            push({});

        for (std::thread& worker : workers)
            worker.join();
    }

    std::function<void()> pop()
    {
        std::unique_lock guard(mtx);

        cv.wait(
            guard,
            [this]
            {
                return !tasks.empty();
            }
        );

        std::function<void()> task = tasks.front();
        tasks.pop();
        return task;
    }

    void push(std::function<void()> task)
    {
        {
            std::unique_lock guard(mtx);
            tasks.push(std::move(task));
        }

        cv.notify_one();
    }

    static unsigned getThreadCount()
    {
        return std::max(std::thread::hardware_concurrency(), 1u);
    }

private:
    void workerFunction()
    {
        while (std::function<void()> task = pop())
            task();
    }

    unsigned threadCount = 1;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
};



#define NUM_INT_OPTS 38
#define NUM_BOOL_OPTS 81

static char const* ZigLuauIntFlagNames[NUM_INT_OPTS] = {
    "LuauSimplificationComplexityLimit",
    "LuauTypeFamilyGraphReductionMaximumSteps",
    "LuauTypeFamilyApplicationCartesianProductLimit",
    "LuauTypeFamilyUseGuesserDepth",
    "LuauTypeFunctionSerdeIterationLimit",
    "LuauTypePathMaximumTraverseSteps",
    "LuauAnySummaryRecursionLimit",
    "LuauTypeCloneIterationLimit",
    "LuauSolverRecursionLimit",
    "LuauIndentTypeMismatchMaxTypeLength",
    "LuauSuggestionDistance",
    "LuauNormalizeCacheLimit",
    "LuauNormalizeIntersectionLimit",
    "LuauTarjanChildLimit",
    "LuauTarjanPreallocationSize",
    "DebugLuauVerboseTypeNames",
    "LuauTypeMaximumStringifierLength",
    "LuauTableTypeMaximumStringifierLength",
    "LuauTypeInferRecursionLimit",
    "LuauTypeInferPackLoopLimit",
    "LuauCheckRecursionLimit",
    "LuauVisitRecursionLimit",
    "LuauRecursionLimit",
    "LuauTypeLengthLimit",
    "LuauParseErrorLimit",
    "CodegenHeuristicsInstructionLimit",
    "CodegenHeuristicsBlockLimit",
    "CodegenHeuristicsBlockInstructionLimit",
    "LuauCodeGenBlockSize",
    "LuauCodeGenMaxTotalSize",
    "LuauCodeGenMinLinearBlockPath",
    "LuauCodeGenReuseSlotLimit",
    "LuauCodeGenReuseUdataTagLimit",
    "LuauCompileLoopUnrollThreshold",
    "LuauCompileLoopUnrollThresholdMaxBoost",
    "LuauCompileInlineThreshold",
    "LuauCompileInlineThresholdMaxBoost",
    "LuauCompileInlineDepth",
};

static char const* ZigLuauBoolFlagNames[NUM_BOOL_OPTS] = {
    "LuauRunCustomModuleChecks",
    "LuauCoroCheckStack",
    "LuauDebugInfoInvArgLeftovers",
    "LuauStackLimit",
    "StudioReportLuauAny2",
    "LuauDocumentationAtPosition",
    "AutocompleteRequirePathSuggestions2",
    "LuauAutocompleteRefactorsForIncrementalAutocomplete",
    "LuauTypestateBuiltins2",
    "LuauStringFormatArityFix",
    "LuauDontRefCountTypesInTypeFunctions",
    "LuauNewSolverVisitExprLvalues",
    "LuauNewSolverPrePopulateClasses",
    "LuauUserTypeFunExportedAndLocal",
    "LuauNewSolverPopulateTableLocations",
    "DebugLuauLogSolver",
    "DebugLuauLogSolverIncludeDependencies",
    "DebugLuauLogBindings",
    "LuauRemoveNotAnyHack",
    "DebugLuauEqSatSimplification",
    "LuauAllowNilAssignmentToIndexer",
    "LuauVectorDefinitions",
    "DebugLuauLogSimplification",
    "DebugLuauLogSimplificationToDot",
    "DebugLuauExtraEqSatSanityChecks",
    "LuauKnowsTheDataModel3",
    "LuauStoreCommentsForDefinitionFiles",
    "DebugLuauLogSolverToJson",
    "DebugLuauLogSolverToJsonFile",
    "DebugLuauForbidInternalTypes",
    "DebugLuauForceStrictMode",
    "DebugLuauForceNonStrictMode",
    "LuauStoreSolverTypeOnModule",
    "LintRedundantNativeAttribute",
    "LuauUserTypeFunNonstrict",
    "LuauCountSelfCallsNonstrict",
    "DebugLuauCheckNormalizeInvariant",
    "LuauNormalizationTracksCyclicPairsThroughInhabitance",
    "LuauIntersectNormalsNeedToTrackResourceLimits",
    "LuauFlagBasicIntersectFollows",
    "DebugLuauSubtypingCheckPathValidity",
    "LuauRetrySubtypingWithoutHiddenPack",
    "LuauSymbolEquality",
    "LuauSyntheticErrors",
    "DebugLuauToStringNoLexicalSort",
    "DebugLuauFreezeArena",
    "LuauTableKeysAreRValues",
    "DebugLuauLogTypeFamilies",
    "LuauUserDefinedTypeFunctionResetState",
    "LuauUserTypeFunFixRegister",
    "LuauUserTypeFunFixNoReadWrite",
    "LuauUserTypeFunFixMetatable",
    "DebugLuauMagicTypes",
    "DebugLuauFreezeDuringUnification",
    "LuauMetatableFollow",
    "LuauRequireCyclesDontAlwaysReturnAny",
    "LuauInstantiateInSubtyping",
    "LuauTransitiveSubtyping",
    "LuauFixIndexerSubtypingOrdering",
    "LuauUnifierRecursionOnRestart",
    "LuauSolver2",
    "LuauUserDefinedTypeFunctionsSyntax2",
    "LuauUserDefinedTypeFunParseExport",
    "LuauAllowFragmentParsing",
    "LuauPortableStringZeroCheck",
    "LuauAllowComplexTypesInGenericParams",
    "LuauErrorRecoveryForTableTypes",
    "DebugLuauTimeTracing",
    "DebugCodegenNoOpt",
    "DebugCodegenOptSize",
    "DebugCodegenSkipNumbering",
    "DebugCodegenChaosA64",
    "LuauVectorLibNativeCodegen",
    "LuauVectorLibNativeDot",
    "DebugLuauAbortingChecks",
    "LuauVectorBuiltins",
    "LuauCompileOptimizeRevArith",
    "LuauCompileVectorTypeInfo",
    "LuauMathMap",
    "LuauVectorMetatable",
    "DebugLuauForceAllNewSolverTests",
};


static void setLuauFlag(std::string_view name, bool state)
{
    for (Luau::FValue<bool>* flag = Luau::FValue<bool>::list; flag; flag = flag->next)
    {
        if (name == flag->name)
        {
            flag->value = state;
            return;
        }
    }

    fprintf(stderr, "Warning: unrecognized flag '%.*s'.\n", int(name.length()), name.data());
}

static void copyBools(ZigLuauBoolFlag* flags) {
    for (int i = 0; i < NUM_BOOL_OPTS; i++) {
        switch (flags[i]) {
            case ZLA_OFF: setLuauFlag(ZigLuauBoolFlagNames[i], false); break;
            case ZLA_ON: setLuauFlag(ZigLuauBoolFlagNames[i], true); break;
            default: break;
        }
    }
}



static void setLuauInt(std::string_view name, int state)
{
    for (Luau::FValue<int>* flag = Luau::FValue<int>::list; flag; flag = flag->next)
    {
        if (name == flag->name)
        {
            flag->value = state;
            return;
        }
    }

    fprintf(stderr, "Warning: unrecognized flag '%.*s'.\n", int(name.length()), name.data());
}

static void copyInts(ZigLuauIntFlag* ints) {
    for (int i = 0; i < NUM_INT_OPTS; i++) {
        if (ints[i].isSet) {
            setLuauInt(ZigLuauIntFlagNames[i], ints[i].value);
        }
    }
}

extern "C" ZigLuauAnalysisContext zig_luau_create_analysis_context(void* userstate) {
    ZigLuauAnalysisContext ctx = {0};
    ctx.userstate = userstate;
    return ctx;
}

extern "C" int zig_luau_analysis(ZigLuauAnalysisContext* zla_ctx, int numRootFiles, char const* const* rootFiles)
{
    Luau::assertHandler() = assertionHandler;

    for (Luau::FValue<bool>* flag = Luau::FValue<bool>::list; flag; flag = flag->next)
        if (strncmp(flag->name, "Luau", 4) == 0 && !Luau::isFlagExperimental(flag->name))
            flag->value = true;
    
    copyInts(&zla_ctx->flags.LuauSimplificationComplexityLimit);
    copyBools(&zla_ctx->flags.LuauRunCustomModuleChecks);

    #if !defined(LUAU_ENABLE_TIME_TRACE)
        if (FFlag::DebugLuauTimeTracing)
        {
            report(zla_ctx, "[Host]", Luau::Location{}, "Notice", "Time tracing requested in flags; not enabled in this build");
            return 1;
        }
    #endif

    Luau::FrontendOptions frontendOptions;
    frontendOptions.retainFullTypeGraphs = zla_ctx->annotateFn != nullptr;
    frontendOptions.runLintChecks = true;

    Luau::Frontend frontend = Luau::Frontend(new FileResolver(zla_ctx), new ConfigResolver(zla_ctx), frontendOptions);
    zla_ctx->frontend = &frontend;

    if (zla_ctx->flags.DebugLuauLogSolverToJsonFile == ZLA_ON)
    {
        frontend.writeJsonLog = [zla_ctx](const Luau::ModuleName& moduleName, std::string log)
        {
            if (zla_ctx->jsonLogFn) zla_ctx->jsonLogFn(zla_ctx->userstate, moduleName.c_str(), log.c_str());
        };
    }

    Luau::registerBuiltinGlobals(frontend, frontend.globals);
    Luau::freeze(frontend.globals.globalTypes);

    std::vector<std::string> files;

    for (int i = 0; i < numRootFiles; i++) {
        files.push_back(rootFiles[i]);
    }

    for (const std::string& path : files) {
        frontend.queueModuleCheck(path);
    }

    std::vector<Luau::ModuleName> checkedModules;

    if (zla_ctx->threadCount <= 0) {
        zla_ctx->threadCount = std::min(TaskScheduler::getThreadCount(), 8u);
        report(zla_ctx, "[Host]", Luau::Location{}, "Notice", "analysis threads requested <1; using default number of threads");
    }

    try
    {
        TaskScheduler scheduler(zla_ctx->threadCount);

        checkedModules = frontend.checkQueuedModules(
            std::nullopt,
            [&](std::function<void()> f)
            {
                scheduler.push(std::move(f));
            }
        );
    }
    catch (const Luau::InternalCompilerError& ice)
    {
        Luau::Location location = ice.location ? *ice.location : Luau::Location();

        std::string moduleName = ice.moduleName ? *ice.moduleName : "<unknown module>";
        std::string humanReadableName = frontend.fileResolver->getHumanReadableModuleName(moduleName);

        Luau::TypeError error(location, moduleName, Luau::InternalError{ice.message});

        report(
            zla_ctx,
            humanReadableName.c_str(),
            location,
            "InternalCompilerError",
            Luau::toString(error, Luau::TypeErrorToStringOptions{frontend.fileResolver}).c_str()
        );

        zla_ctx->frontend = nullptr;

        return -1;
    }

    int failed = 0;

    for (const Luau::ModuleName& name : checkedModules)
        failed += !reportModuleResult(zla_ctx, name);

    zla_ctx->frontend = nullptr;

    return failed;
}
