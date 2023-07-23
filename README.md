# fakeyou.cpp

## Features
- Authentication
- Ratelimit safe
- Fetch profile data
- TTS functionality
- View the tts/w2l leaderboard

## Requirements
- [cURL](https://curl.se/)
- [nlohmann/json](https://github.com/nlohmann/json)

## Example Usage
```cpp
#include "Client.hpp"

int main()
{
	FakeYou::Client* client = new FakeYou::Client; // initialize a fakeyou client
	
	// - Authentication
	
	client->Login("<username>", "<password>"); // username and password you signed up with
	
	client->Logout(); // logout at anytime to go back to being represented as anonymous

	// - Miscellaneous

	std::vector<FakeYou::LeaderboardEntry*> ttsLeaderboard = client->FetchLeaderboard()->ttsLeaderboard; // returns a sloppy user, but you can refetch the user by username you're provided
	std::vector<FakeYou::LeaderboardEntry*> w2lLeaderboard = client->FetchLeaderboard()->w2lLeaderboard; // returns a sloppy user, but you can refetch the user by username you're provided
	FakeYou::Profile* topProfile = client->FetchProfile(ttsLeaderboard[0]->username);

	std::cout << topProfile->patreonUsername << std::endl;

	// - Text to speech

	FakeYou::TTSModel* modelByTitle = client->tts->GetModelByTitle("Saul Goodman"); // fetch a model object by title
	FakeYou::TTSModel* modelByToken = client->tts->GetModelByToken("TM:b8efnnxdx14m"); // you can also fetch by token
	std::vector<FakeYou::TTSModel*> models = client->tts->GetModels(); // or just fetch them all

	FakeYou::TTSResult* result = modelByTitle->Inference("I'm over here stroking my dick I got lotion on my dick right now I'm just stroking my shit I'm horny as fuck man I'm a freak man like");

	std::cout << result->url << std::endl;
	result->Download("./", "pimento_cheese_sandwich"); // Save the wav from the url file to disk

	return 0;
}
```

## Compile with g++
```
g++ *.cpp -lcurl
```
