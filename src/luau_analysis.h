#ifndef luau_analysis_h
#define luau_analysis_h

#include <stdbool.h>


typedef enum {
    ZLA_NONSTRICT,
    ZLA_STRICT,
    ZLA_DEFINITION,
} ZigLuauMode;

typedef struct {
    unsigned int beginLine;
    unsigned int beginColumn;
    unsigned int endLine;
    unsigned int endColumn;
} ZigLuauLocation;

typedef enum {
    ZLA_DEFAULT = 0,
    ZLA_OFF = 1,
    ZLA_ON = 2,
} ZigLuauBoolFlag;

typedef struct {
    bool isSet;
    int value;
} ZigLuauIntFlag;


typedef struct {
    bool Unknown;
    bool UnknownGlobal;
    bool DeprecatedGlobal;
    bool GlobalUsedAsLocal;
    bool LocalShadow;
    bool SameLineStatement;
    bool MultiLineStatement;
    bool LocalUnused;
    bool FunctionUnused;
    bool ImportUnused;
    bool BuiltinGlobalWrite;
    bool PlaceholderRead;
    bool UnreachableCode;
    bool UnknownType;
    bool ForRange;
    bool UnbalancedAssignment;
    bool ImplicitReturn;
    bool DuplicateLocal;
    bool FormatString;
    bool TableLiteral;
    bool UninitializedLocal;
    bool DuplicateFunction;
    bool DeprecatedApi;
    bool TableOperations;
    bool DuplicateCondition;
    bool MisleadingAndOr;
    bool CommentDirective;
    bool IntegerParsing;
    bool ComparisonPrecedence;
    bool RedundantNativeAttribute;
} ZigLuauLintOptions;


typedef struct {
    ZigLuauMode mode;

    bool allowDeclarationSyntax;
    bool captureComments;

    ZigLuauLintOptions enabledLint;
    ZigLuauLintOptions fatalLint;

    bool lintErrors;
    bool typeErrors;

    int numGlobals;
    const char** globals;
} ZigLuauConfig;

typedef struct {
    ZigLuauIntFlag LuauSimplificationComplexityLimit;
    ZigLuauIntFlag LuauTypeFamilyGraphReductionMaximumSteps;
    ZigLuauIntFlag LuauTypeFamilyApplicationCartesianProductLimit;
    ZigLuauIntFlag LuauTypeFamilyUseGuesserDepth;
    ZigLuauIntFlag LuauTypeFunctionSerdeIterationLimit;
    ZigLuauIntFlag LuauTypePathMaximumTraverseSteps;

    ZigLuauIntFlag LuauAnySummaryRecursionLimit;
    ZigLuauIntFlag LuauTypeCloneIterationLimit;
    ZigLuauIntFlag LuauSolverRecursionLimit;
    ZigLuauIntFlag LuauIndentTypeMismatchMaxTypeLength;
    ZigLuauIntFlag LuauSuggestionDistance;
    ZigLuauIntFlag LuauNormalizeCacheLimit;
    ZigLuauIntFlag LuauNormalizeIntersectionLimit;
    ZigLuauIntFlag LuauTarjanChildLimit;
    ZigLuauIntFlag LuauTarjanPreallocationSize;
    ZigLuauIntFlag DebugLuauVerboseTypeNames;
    ZigLuauIntFlag LuauTypeMaximumStringifierLength;
    ZigLuauIntFlag LuauTableTypeMaximumStringifierLength;
    ZigLuauIntFlag LuauTypeInferRecursionLimit;
    ZigLuauIntFlag LuauTypeInferPackLoopLimit;
    ZigLuauIntFlag LuauCheckRecursionLimit;
    ZigLuauIntFlag LuauVisitRecursionLimit;
    ZigLuauIntFlag LuauRecursionLimit;
    ZigLuauIntFlag LuauTypeLengthLimit;
    ZigLuauIntFlag LuauParseErrorLimit;
    ZigLuauIntFlag CodegenHeuristicsInstructionLimit;
    ZigLuauIntFlag CodegenHeuristicsBlockLimit;
    ZigLuauIntFlag CodegenHeuristicsBlockInstructionLimit;
    ZigLuauIntFlag LuauCodeGenBlockSize;
    ZigLuauIntFlag LuauCodeGenMaxTotalSize;
    ZigLuauIntFlag LuauCodeGenMinLinearBlockPath;
    ZigLuauIntFlag LuauCodeGenReuseSlotLimit;
    ZigLuauIntFlag LuauCodeGenReuseUdataTagLimit;
    ZigLuauIntFlag LuauCompileLoopUnrollThreshold;
    ZigLuauIntFlag LuauCompileLoopUnrollThresholdMaxBoost;
    ZigLuauIntFlag LuauCompileInlineThreshold;
    ZigLuauIntFlag LuauCompileInlineThresholdMaxBoost;
    ZigLuauIntFlag LuauCompileInlineDepth;
    
    ZigLuauBoolFlag LuauRunCustomModuleChecks;
    ZigLuauBoolFlag LuauCoroCheckStack;
    ZigLuauBoolFlag LuauDebugInfoInvArgLeftovers;
    ZigLuauBoolFlag LuauStackLimit;

    ZigLuauBoolFlag StudioReportLuauAny2;
    ZigLuauBoolFlag LuauDocumentationAtPosition;
    ZigLuauBoolFlag AutocompleteRequirePathSuggestions2;
    ZigLuauBoolFlag LuauAutocompleteRefactorsForIncrementalAutocomplete;
    ZigLuauBoolFlag LuauTypestateBuiltins2;
    ZigLuauBoolFlag LuauStringFormatArityFix;
    ZigLuauBoolFlag LuauDontRefCountTypesInTypeFunctions;
    ZigLuauBoolFlag LuauNewSolverVisitExprLvalues;
    ZigLuauBoolFlag LuauNewSolverPrePopulateClasses;
    ZigLuauBoolFlag LuauUserTypeFunExportedAndLocal;
    ZigLuauBoolFlag LuauNewSolverPopulateTableLocations;
    ZigLuauBoolFlag DebugLuauLogSolver;
    ZigLuauBoolFlag DebugLuauLogSolverIncludeDependencies;
    ZigLuauBoolFlag DebugLuauLogBindings;
    ZigLuauBoolFlag LuauRemoveNotAnyHack;
    ZigLuauBoolFlag DebugLuauEqSatSimplification;
    ZigLuauBoolFlag LuauAllowNilAssignmentToIndexer;
    ZigLuauBoolFlag LuauVectorDefinitions;
    ZigLuauBoolFlag DebugLuauLogSimplification;
    ZigLuauBoolFlag DebugLuauLogSimplificationToDot;
    ZigLuauBoolFlag DebugLuauExtraEqSatSanityChecks;
    ZigLuauBoolFlag LuauKnowsTheDataModel3;
    ZigLuauBoolFlag LuauStoreCommentsForDefinitionFiles;
    ZigLuauBoolFlag DebugLuauLogSolverToJson;
    ZigLuauBoolFlag DebugLuauLogSolverToJsonFile;
    ZigLuauBoolFlag DebugLuauForbidInternalTypes;
    ZigLuauBoolFlag DebugLuauForceStrictMode;
    ZigLuauBoolFlag DebugLuauForceNonStrictMode;
    ZigLuauBoolFlag LuauStoreSolverTypeOnModule;
    ZigLuauBoolFlag LintRedundantNativeAttribute;
    ZigLuauBoolFlag LuauUserTypeFunNonstrict;
    ZigLuauBoolFlag LuauCountSelfCallsNonstrict;
    ZigLuauBoolFlag DebugLuauCheckNormalizeInvariant;
    ZigLuauBoolFlag LuauNormalizationTracksCyclicPairsThroughInhabitance;
    ZigLuauBoolFlag LuauIntersectNormalsNeedToTrackResourceLimits;
    ZigLuauBoolFlag LuauFlagBasicIntersectFollows;
    ZigLuauBoolFlag DebugLuauSubtypingCheckPathValidity;
    ZigLuauBoolFlag LuauRetrySubtypingWithoutHiddenPack;
    ZigLuauBoolFlag LuauSymbolEquality;
    ZigLuauBoolFlag LuauSyntheticErrors;
    ZigLuauBoolFlag DebugLuauToStringNoLexicalSort;
    ZigLuauBoolFlag DebugLuauFreezeArena;
    ZigLuauBoolFlag LuauTableKeysAreRValues;
    ZigLuauBoolFlag DebugLuauLogTypeFamilies;
    ZigLuauBoolFlag LuauUserDefinedTypeFunctionResetState;
    ZigLuauBoolFlag LuauUserTypeFunFixRegister;
    ZigLuauBoolFlag LuauUserTypeFunFixNoReadWrite;
    ZigLuauBoolFlag LuauUserTypeFunFixMetatable;
    ZigLuauBoolFlag DebugLuauMagicTypes;
    ZigLuauBoolFlag DebugLuauFreezeDuringUnification;
    ZigLuauBoolFlag LuauMetatableFollow;
    ZigLuauBoolFlag LuauRequireCyclesDontAlwaysReturnAny;
    ZigLuauBoolFlag LuauInstantiateInSubtyping;
    ZigLuauBoolFlag LuauTransitiveSubtyping;
    ZigLuauBoolFlag LuauFixIndexerSubtypingOrdering;
    ZigLuauBoolFlag LuauUnifierRecursionOnRestart;
    ZigLuauBoolFlag LuauSolver2;
    ZigLuauBoolFlag LuauUserDefinedTypeFunctionsSyntax2;
    ZigLuauBoolFlag LuauUserDefinedTypeFunParseExport;
    ZigLuauBoolFlag LuauAllowFragmentParsing;
    ZigLuauBoolFlag LuauPortableStringZeroCheck;
    ZigLuauBoolFlag LuauAllowComplexTypesInGenericParams;
    ZigLuauBoolFlag LuauErrorRecoveryForTableTypes;
    ZigLuauBoolFlag DebugLuauTimeTracing;
    ZigLuauBoolFlag DebugCodegenNoOpt;
    ZigLuauBoolFlag DebugCodegenOptSize;
    ZigLuauBoolFlag DebugCodegenSkipNumbering;
    ZigLuauBoolFlag DebugCodegenChaosA64;
    ZigLuauBoolFlag LuauVectorLibNativeCodegen;
    ZigLuauBoolFlag LuauVectorLibNativeDot;
    ZigLuauBoolFlag DebugLuauAbortingChecks;
    ZigLuauBoolFlag LuauVectorBuiltins;
    ZigLuauBoolFlag LuauCompileOptimizeRevArith;
    ZigLuauBoolFlag LuauCompileVectorTypeInfo;
    ZigLuauBoolFlag LuauMathMap;
    ZigLuauBoolFlag LuauVectorMetatable;
    ZigLuauBoolFlag DebugLuauForceAllNewSolverTests;
} ZigLuauFlags;


typedef void (*ZigLuauExternReportFn) (void* userstate, const char* moduleName, ZigLuauLocation loc, const char* type, const char* message);
typedef void (*ZigLuauExternAnnotateFn) (void* userstate, const char* moduleName, const char* annotatedSource);
typedef bool (*ZigLuauExternCacheManagerFn) (void* userstate, const char* path);
typedef char const* (*ZigLuauExternGetRequireSourceTextFn) (void* userstate, const char* path);
typedef void (*ZigLuauExternConfigResolverFn) (void* userstate, ZigLuauConfig* config);
typedef void (*ZigLuauExternJsonLogFn)(void* userstate, char const* moduleName, char const* log);

typedef struct {
    int threadCount;

    void* userstate;

    ZigLuauExternReportFn reportFn;
    ZigLuauExternAnnotateFn annotateFn;
    ZigLuauExternCacheManagerFn cacheManagerFn;
    ZigLuauExternGetRequireSourceTextFn getRequireSourceTextFn;
    ZigLuauExternConfigResolverFn configResolverFn;
    ZigLuauExternJsonLogFn jsonLogFn;
    
    void* frontend;

    ZigLuauFlags flags;
    ZigLuauConfig config;
} ZigLuauAnalysisContext;

extern int zig_luau_analysis(ZigLuauAnalysisContext* zla_ctx, int numRootFiles, char const* const* rootFiles);

extern ZigLuauAnalysisContext zig_luau_create_analysis_context(void* userstate);

#endif // luau_analysis_h
