//code from https://forums.unrealengine.com/t/function-to-get-all-filenames-in-a-directory/25098/3

template <class FunctorType>
class PlatformFileFunctor : public IPlatformFile::FDirectoryVisitor	//GenericPlatformFile.h
{
public:
	
	virtual bool Visit(const TCHAR* filenameOrDirectory, bool isDirectory) override
	{
		return functor(filenameOrDirectory, isDirectory);
	}

	PlatformFileFunctor(FunctorType&& functorInstance)
		: functor(MoveTemp(functorInstance))
	{
	}

private:
	FunctorType functor;
};

template <class Functor>
PlatformFileFunctor<Functor> MakeDirectoryVisitor(Functor&& functorInstance)
{
	return PlatformFileFunctor<Functor>(MoveTemp(functorInstance));
}

static FORCEINLINE bool GetFiles(const FString& fullPathOfBaseDir, TArray<FString>& filenamesOut, bool recursive=false, const FString& filterByExtension = "")
{
	//Format File Extension, remove the "." if present
	const FString fileExtension = filterByExtension.Replace(TEXT("."),TEXT("")).ToLower();
	
	FString string;
	auto filenamesVisitor = MakeDirectoryVisitor([&](const TCHAR* filenameOrDirectory, bool isDirectory) 
		{
			//Files
			if (!isDirectory)
			{
				//Filter by Extension
				if(fileExtension != "")
				{
					string = FPaths::GetCleanFilename(filenameOrDirectory);
				
					//Filter by Extension
					if(FPaths::GetExtension(string).ToLower() == fileExtension) 
					{
						if(recursive) 
						{
							filenamesOut.Push(filenameOrDirectory); //need whole path for recursive
						}
						else 
						{
							filenamesOut.Push(string);
						}
					}
				}
				
				//Include All Filenames!
				else
				{
					//Just the Directory
					string = FPaths::GetCleanFilename(filenameOrDirectory);
					
					if(recursive) 
					{
						filenamesOut.Push(filenameOrDirectory); //need whole path for recursive
					}
					else 
					{
						filenamesOut.Push(string);
					}
				}
			}
			return true;
		}
	);
	if(recursive) 
	{
		return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*fullPathOfBaseDir, filenamesVisitor);
	}
	else 
	{
		return FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*fullPathOfBaseDir, filenamesVisitor);
	}
}	