// Cosmic Ascension - automation tests of the data pipeline. Design: guideline.md section 9.3, 10.1.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Tests/CosmicTestHelpers.h"

namespace CosmicDataTests
{
	// SHA-1 of a file without its carriage returns, so that the line ending conversion of a checkout does not matter.
	// Tools/make_data_tables.py hashes the same way. Empty when the file cannot be read.
	FString HashOfSource(const FString& Path)
	{
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *Path))
		{
			return FString();
		}
		Bytes.RemoveAll([](uint8 Byte) { return Byte == 0x0D; });
		return FSHA1::HashBuffer(Bytes.GetData(), static_cast<uint64>(Bytes.Num())).ToString();
	}
}

// The JSON and CSV files under Tools/data are the source of the DataTable and String Table assets, and the game reads only
// the assets. The script that generates them leaves the hashes of its sources behind; a source edited afterwards without
// running the script again fails here.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDataSourcesInSyncTest, "Cosmic_Ascension.Data.SourcesInSync", CosmicTests::Flags)
bool FCosmicDataSourcesInSyncTest::RunTest(const FString& Parameters)
{
	const FString SourceDir = FPaths::ProjectDir() / TEXT("Tools/data");
	if (!IFileManager::Get().DirectoryExists(*SourceDir))
	{
		AddInfo(TEXT("No Tools/data folder next to this build: nothing to compare."));
		return true;
	}

	TArray<FString> Lines;
	if (!TestTrue(TEXT("Tools/data/generated.sha1 can be read (run Tools/make_data_tables.py)"), FFileHelper::LoadFileToStringArray(Lines, *(SourceDir / TEXT("generated.sha1")))))
	{
		return false;
	}

	// One line per source the script turned into an asset: "<file name> <sha1>".
	TMap<FString, FString> Recorded;
	for (const FString& Line : Lines)
	{
		FString FileName;
		FString Hash;
		if (Line.TrimStartAndEnd().Split(TEXT(" "), &FileName, &Hash))
		{
			Recorded.Add(FileName, Hash.TrimStartAndEnd());
		}
	}
	TestTrue(TEXT("The stamp lists at least one source"), Recorded.Num() > 0);

	for (const TPair<FString, FString>& Entry : Recorded)
	{
		const FString Current = CosmicDataTests::HashOfSource(SourceDir / Entry.Key);
		TestTrue(*FString::Printf(TEXT("%s is unchanged since the assets were generated (run Tools/make_data_tables.py)"), *Entry.Key),
			!Current.IsEmpty() && Current.Equals(Entry.Value, ESearchCase::IgnoreCase));
	}

	// A source the script does not know is a table nobody generates.
	for (const TCHAR* Extension : { TEXT("json"), TEXT("csv") })
	{
		TArray<FString> Found;
		IFileManager::Get().FindFiles(Found, *SourceDir, Extension);
		for (const FString& FoundPath : Found)
		{
			// In 5.8 this overload gives file names only; GetCleanFilename keeps the line right for either form.
			const FString FileName = FPaths::GetCleanFilename(FoundPath);
			TestTrue(*FString::Printf(TEXT("%s is listed in the stamp (add it to Tools/make_data_tables.py)"), *FileName), Recorded.Contains(FileName));
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
