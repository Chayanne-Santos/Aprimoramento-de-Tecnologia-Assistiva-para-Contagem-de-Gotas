#ifndef BTN_H_INCLUDED
#define BTN_H_INCLUDED

#include <Arduino.h>

class Btn{
  public:
    int *pino;
    bool btnclicado;
    bool btnliberado;
    int ultimoEstadoBtn=LOW;
    unsigned long tempoPrimeiroAcionamento=0;
    unsigned long tempoDebounce=50;

    Btn (int p);
    typedef void (funcao)(void);
    void clique (funcao *f);
 
};

#endif // BTN_H_INCLUDED
