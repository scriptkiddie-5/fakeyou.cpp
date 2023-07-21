# FakeYou CPP (WIP)

## Requirements
[cURL](https://curl.se/)

## Example Usage
```cpp
#include "Client.hpp"

int main()
{
  // Initialize client instance
  FakeYou::Client client;

  // You can choose not do do this and be represented as an anonymous user
  client.Login("<username>", "<password>"); 

  // Fetch a model by token
  FakeYou::TTSModel* model = client.GetModelByToken("TM:4t2a9skxk1v8");
  // Infer text to speech
  FakeYou::TTSResult* result = model->Infer("I have just contracted aids from my stuffed animal pikachu.");
  if (result != nullptr)
    // Give converted text to speech url
    std::cout << result->url;

  getchar();
  return 0;
}
