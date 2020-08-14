#include <iostream>
#include <thread>
#include <cstring>
#include <fstream>
#include <queue>
#include <unistd.h>
#include <cpprest/http_client.h>

using namespace std;
using namespace web::http;
using namespace web::http::client;

uri_builder create_uri(string playlist_id, string apikey, string token = "") {
    uri_builder uri;
    uri.set_scheme("https");
    uri.set_host("www.googleapis.com");
    uri.set_path("youtube/v3/playlistItems");
    uri.set_query("part=snippet");
    uri.append_query("maxResults=50");
    uri.append_query("playlistId=" + playlist_id);
    uri.append_query("key=" + apikey);
    uri.append_query("pageToken=" + token);
    return uri;
}

string create_command(string video_id) {
    string base = "youtube-dl -x --audio-format \"mp3\" --output \'%(title)s.%(ext)s\' https://www.youtube.com/watch?v=";
    return (base+video_id);
}

void download_function(queue<string>* container) {
    int size = container->size();
    for (int i = 0; i < size; i++) {
        string tmp = container->front(); container->pop();
        // cout << tmp << endl;
        system(tmp.c_str());
    }
}

int main(int argc, char** argv) {
    const int thread_count = std::thread::hardware_concurrency();
    string apikey;
    string p_id;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--apikey")) {
            if (i < argc - 1) {
                i++;
            } else {
                cout << "--apikey is specified but no value entered. " << endl;
                return -1;
            }
            apikey = string(argv[i]);
            ifstream tmp_file(apikey);
            if (tmp_file.is_open()) {
                // Meaning input was file path
                getline(tmp_file, apikey);
            }
        } else if (!strcmp(argv[i], "--play")) {
            if (i < argc - 1) {
                i++;
            } else {
                cout << "--play is specified but no value entered" << endl;
            }
            p_id = string(argv[i]);
            if (p_id.find("https://www.youtube.com/playlist?list=") != string::npos) {
                int location = p_id.find("https://www.youtube.com/playlist?list=");
                p_id = p_id.substr(location+38, p_id.length());
            }
        }
    }
    string token = "";
    http_request request_tpr (methods::GET);
    queue<string> queue_for_run[thread_count];
    int count = 0;
    while (true) {
        count = count % thread_count;
        http_client client(create_uri(p_id, apikey, token).to_string());
        web::json::value root_value;
        try {
            client.request(request_tpr).then([&root_value] (http_response hr) {
                root_value = hr.extract_json().get();
            }).wait();
        } catch (const exception& expn) {
            cout << expn.what() << endl;
        }

        int item_size = root_value["items"].size();
        for (int i = 0; i < item_size; i++) {
            count = count % thread_count;
            queue_for_run[count++].push(create_command(root_value["items"][i]["snippet"]["resourceId"]["videoId"].as_string()));
        }
        if (!root_value["nextPageToken"].is_null()) {
            token = root_value["nextPageToken"].as_string();
        } else {
            break;
        }
    }

    // int sum = 0;
    // for (int i = 0; i < thread_count; i++) {
    //     sum += queue_for_run[i].size();
    // }
    // cout << sum << endl;
    for (int i = 0; i < thread_count; i++) {
        pid_t t_f = fork();
        if (t_f == 0) {
            cout << "Working for " << i << endl;
            download_function(&queue_for_run[i]);
            //sleep(5);
            exit(0);
        }
    }
    for(int i=0;i<thread_count;i++)
        wait(NULL); 
    return 0;
}