//
// Created by sunjjinbo on 2022/1/14.
//

#ifndef MAZE_CARDBOARDAPP_H
#define MAZE_CARDBOARDAPP_H

class CardboardApp {
public:
    CardboardApp(JavaVM* vm, jobject obj);
    ~CardboardApp();

    void SetScreenParams(int width, int height);
    void OnDrawFrame();
    void OnPause();
    void OnResume();
    void SwitchViewer();
};

#endif //MAZE_CARDBOARDAPP_H
