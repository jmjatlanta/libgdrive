#include "gdrive/gdrive.hpp"
#include <iostream>
#include <assert.h>
#include <vector>

using namespace GDRIVE;

int main() {
    char* user_home = getenv("HOME");
    if (user_home == NULL) {
        fprintf(stderr, "No $HOME environment variable\n");
        exit(-1);
    }

    char default_gdrive_dir[512];
    strcpy(default_gdrive_dir, user_home);
    strcat(default_gdrive_dir, "/.gdrive/data");

    char* gdrive_dir = getenv("GDRIVE");
    if (gdrive_dir == NULL) {
        gdrive_dir = default_gdrive_dir;
    }
    
    FileStore fs(gdrive_dir);
    assert(fs.status() == SS_FULL);

    Credential cred(&fs);

    if (fs.get("refresh_token") == "") {
        std::string client_id = fs.get("client_id");
        std::string client_secret = fs.get("client_secret");

        OAuth oauth(client_id, client_secret);    
        std::cout << "Please go to this url using your browser, after you authorize this application, you will get a code from your browser" << std::endl
                  <<oauth.get_authorize_url() << std::endl;
        std::cout << "Please enter the code: ";
        std::string code;
        std::cin >> code;
        oauth.build_credential(code, cred);
    }

    Drive service(&cred);
    std::vector<GChildren> children = service.children().Listall("root");

    for (int i = 0; i < children.size(); i ++ ) {
        std::cout << "Id " << children[i].get_id() << std::endl
                  << "selfLink " << children[i].get_selfLink() << std::endl
                  << "childLink " << children[i].get_childLink() << std::endl << std::endl;
        FileGetRequest get = service.files().Get(children[i].get_id());
        get.add_field("id,title,mimeType");
        GFile file = get.execute();
        if (file.get_mimeType() == "application/vnd.google-apps.folder") {
            std::cout << "[" << file.get_title() << "] is a folder, and it's children are " << std::endl;
            std::vector<GChildren> sub_children = service.children().Listall(file.get_id());
            for(int j = 0; j < sub_children.size(); j ++) {
                std::cout << "\tId " << sub_children[j].get_id() << std::endl;
            }
        }

    }
    children.clear();
}
