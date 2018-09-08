// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomBPNodes.h"
#include "Json.h"
#include "JsonUtilities/Public/JsonUtilities.h"

//#include "PerfCounters\Public\PerfCountersModule.h"
#include "SharedPointer.h"
#include <sstream>

void UCustomBPNodes::GetBytesFromJSONString(FString JsonString, TArray<uint8>& bytes)
{
	std::string str = TCHAR_TO_UTF8(*JsonString);
	for (int i = 0; i < str.size(); i++)
	{
		bytes.Add(str[i]);
	}
}

void UCustomBPNodes::HandleMindwaveResponse(FString message, TMap<FString, bool>& boolMaps, TMap<FString, FString>& stringMaps, TMap<FString, float>& numberMaps, FString& debugOut)
{
	std::stringstream outStream;
	std::string totalString;
	/*for (int i = 0; i < bytes.Num(); i++)
	{
		totalString += bytes[i];
	}*/
	
	FString JsonStr = message;

	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
	EJsonNotation valueType;
	while (JsonReader->ReadNext(valueType))
	{
		if (JsonReader->GetIdentifier() == "")
		{
			continue;
		}
		outStream << "Identitfier: " << TCHAR_TO_UTF8(*JsonReader->GetIdentifier());
		if (valueType == EJsonNotation::Boolean)
		{
			bool b = JsonReader->GetValueAsBoolean();
			outStream << "bool val: " << b << std::endl;
			boolMaps.Add(JsonReader->GetIdentifier(),b);
		}
		else  if (valueType == EJsonNotation::String)
		{
			FString string = JsonReader->GetValueAsString();
			outStream << "string val: " << TCHAR_TO_UTF8(*string) << std::endl;
			stringMaps.Add(JsonReader->GetIdentifier(), string);
		}
		else if (valueType == EJsonNotation::Number)
		{
			double doubleVal = JsonReader->GetValueAsNumber();
			outStream << "number val: " << doubleVal << std::endl;
			numberMaps.Add(JsonReader->GetIdentifier(), (float)doubleVal);
		}
	}
	debugOut = UTF8_TO_TCHAR(outStream.str().c_str());

}

void UCustomBPNodes::ParseByteArrayToJSON(TArray<uint8> byteArray, FString& output)
{
	std::stringstream outStream;
	std::string totalString;
	for (int i = 0; i < byteArray.Num(); i++)
	{
		totalString += byteArray[i];
	}
	outStream << totalString;
	/*
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("Name", "Super Sword");
	JsonObject->SetNumberField("Damage", 15);
	JsonObject->SetNumberField("Weight", 3);

	FString OutputString;
	
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FString JsonStr;
	
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(OutputString);
	EJsonNotation valueType;
	while (JsonReader->ReadNext(valueType))
	{
		
		if (valueType == EJsonNotation::Boolean)
		{
			bool b = JsonReader->GetValueAsBoolean();
			outStream << "boolean: " << b <<std::endl;
		}
		else  if(valueType == EJsonNotation::String)
		{
			FString string = JsonReader->GetValueAsString();
			outStream << "string: " << TCHAR_TO_UTF8(*string) << std::endl;
		}
		else if (valueType == EJsonNotation::Number)
		{
			double doubleVal = JsonReader->GetValueAsNumber();
			outStream << "number: " << doubleVal << std::endl;
		}
	}
	*/
	output = UTF8_TO_TCHAR(outStream.str().c_str());
}