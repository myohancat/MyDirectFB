#ifndef __GPIO_H_
#define __GPIO_H_

typedef enum {
    GPIO_DIR_IN,
    GPIO_DIR_OUT,

    GPIO_DIR_UNKNOWN
}GPIO_Dir_e;

typedef enum {
    GPIO_EDGE_NONE,
    GPIO_EDGE_RISING,
    GPIO_EDGE_FALLING,
    GPIO_EDGE_BOTH,

    GPIO_EDGE_UNKNOWN
}GPIO_Edge_e;

class GPIO
{
public:
    static GPIO* open(int num);
    virtual ~GPIO() { }

    GPIO_Dir_e   getOutDir();
    void         setOutDir(GPIO_Dir_e dir);

    GPIO_Edge_e  getEdge();
    void         setEdge(GPIO_Edge_e  edge);

    bool         isActiveLow();
    void         setActiveLow(bool activeLow);

    bool         getValue();
    void         setValue(bool value);

    const char*  getPath() { return mPath; }

    int          getNumber() { return mNum; }

private:
    static bool _export(int num);
    static bool _exist(int num);

    GPIO(int num);


private:
    int  mNum;
    char mPath[1024];
};


#endif //__GPIO_H_
