# FakeYou CPP (WIP)

## What can it do
- Authentication
- Profile fetching
- TTS model fetching
- Rate tts models
- TTS synthesis
- Download tts synthesis's

## Requirements
[cURL](https://curl.se/)

[nlohmann/json](https://github.com/nlohmann/json)

## Example Usage
```cpp
#include "Client.hpp"

int main()
{
	FakeYou::Client* client = new FakeYou::Client(); // initialize a fakeyou client
	client->Login("username", "password"); // username and password you signed up with, don't call this function if you want to be represented as anonymous

	FakeYou::Profile* myProfile = client->FetchMyProfile(); // important: this will return nullptr if you're not logged in
	FakeYou::Profile* someProfile = client->FetchProfile("joe_mama"); // fetch a profile by username

	FakeYou::TTSModel* modelByToken = client->tts->GetModelByToken("TM:tzjnqpmep7tn"); // fetch tts model by token
	FakeYou::TTSModel* modelByTitle = client->tts->GetModelByTitle("Mike Ehrmantraut"); // fetch tts model by title

	modelByToken->Refetch(); // you can refetch a model to ensure you have up-to-date information

	FakeYou::TTSModel::Rating modelByTokenRating = modelByToken->GetMyRating(); // the logged in user's rating
	bool good = modelByToken->SetMyRating(FakeYou::TTSModel::Rating::Positive); // set the logged in user's rating

	std::cout << "Username is " << myProfile->username << std::endl;
	std::cout << "Synthesizing" << std::endl;

	FakeYou::TTSResult* result = modelByToken->Inference("I'm extremely tired, god help me."); // the fun part, synthesize some text using your model!
	result->Download("./", "pimento_cheese"); // download the .wav to a path with a name

	std::cout << "Synthesized and downloaded: " << result->url << std::endl;

	getchar();
	return 0;
}
```