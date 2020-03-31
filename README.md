To compile, run

```
g++ -O3 -std=c++11 cache_server.cc -lboost_thread -lboost_system -pthread
```

To start the server (default on port `8080`), run

```
./a.out
```

To test an API call, in another terminal, run the corresponding **curl** command. For example, to test a **GET /key** call to the server, run  `curl http://localhost:8080/{key} --output - -X GET`). See more **curl** examples [here](https://www.codepedia.org/ama/how-to-test-a-rest-api-from-command-line-with-curl/).
