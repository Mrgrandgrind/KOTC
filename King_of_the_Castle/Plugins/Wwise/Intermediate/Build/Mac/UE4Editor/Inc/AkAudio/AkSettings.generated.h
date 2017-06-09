// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	C++ class header boilerplate exported from UnrealHeaderTool.
	This is automatically generated by the tools.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef AKAUDIO_AkSettings_generated_h
#error "AkSettings.generated.h already included, missing '#pragma once' in AkSettings.h"
#endif
#define AKAUDIO_AkSettings_generated_h

#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_RPC_WRAPPERS
#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_RPC_WRAPPERS_NO_PURE_DECLS
#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_INCLASS_NO_PURE_DECLS \
	private: \
	static void StaticRegisterNativesUAkSettings(); \
	friend AKAUDIO_API class UClass* Z_Construct_UClass_UAkSettings(); \
	public: \
	DECLARE_CLASS(UAkSettings, UObject, COMPILED_IN_FLAGS(0 | CLASS_DefaultConfig | CLASS_Config), 0, TEXT("/Script/AkAudio"), NO_API) \
	DECLARE_SERIALIZER(UAkSettings) \
	/** Indicates whether the class is compiled into the engine */ \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC}; \
	static const TCHAR* StaticConfigName() {return TEXT("Game");} \



#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_INCLASS \
	private: \
	static void StaticRegisterNativesUAkSettings(); \
	friend AKAUDIO_API class UClass* Z_Construct_UClass_UAkSettings(); \
	public: \
	DECLARE_CLASS(UAkSettings, UObject, COMPILED_IN_FLAGS(0 | CLASS_DefaultConfig | CLASS_Config), 0, TEXT("/Script/AkAudio"), NO_API) \
	DECLARE_SERIALIZER(UAkSettings) \
	/** Indicates whether the class is compiled into the engine */ \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC}; \
	static const TCHAR* StaticConfigName() {return TEXT("Game");} \



#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UAkSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UAkSettings) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UAkSettings); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UAkSettings); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UAkSettings(UAkSettings&&); \
	NO_API UAkSettings(const UAkSettings&); \
public:


#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UAkSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UAkSettings(UAkSettings&&); \
	NO_API UAkSettings(const UAkSettings&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UAkSettings); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UAkSettings); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UAkSettings)


#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_PRIVATE_PROPERTY_OFFSET
#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_5_PROLOG
#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_PRIVATE_PROPERTY_OFFSET \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_RPC_WRAPPERS \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_INCLASS \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_PRIVATE_PROPERTY_OFFSET \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_RPC_WRAPPERS_NO_PURE_DECLS \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_INCLASS_NO_PURE_DECLS \
	King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h_8_ENHANCED_CONSTRUCTORS \
static_assert(false, "Unknown access specifier for GENERATED_BODY() macro in class AkSettings."); \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID King_of_the_Castle_Plugins_Wwise_Source_AkAudio_Classes_AkSettings_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
