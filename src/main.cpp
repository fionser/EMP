//
// Created by Lu WJ on 6/6/2017 AD.
//

#include "emp/emp"

template<typename IO>
void eval_feed(Backend* be, int party, block * label, const bool*, int length);
template<typename IO>
void eval_reveal(Backend* be, bool* clear, int party, const block * label, int length);

template<typename IO>
class SemiHonestEva: public Backend { public:
    IO* io = nullptr;
    SHOTIterated<IO>* ot;
    HalfGateEva<IO> * gc;
    SemiHonestEva(IO *io, HalfGateEva<IO> * gc): Backend(BOB) {
       this->io = io;
       ot = new SHOTIterated<IO>(io, false);
       this->gc = gc;
       Feed_internal = eval_feed<IO>;
       Reveal_internal = eval_reveal<IO>;
    }
    ~SemiHonestEva() {
       delete ot;
    }
};


template<typename IO>
void eval_feed(Backend* be, int party, block * label, const bool* b, int length) {
   SemiHonestEva<IO> * backend = (SemiHonestEva<IO>*)(be);
   if(party == ALICE) {
      backend->io->recv_block(label, length);
   } else {
      backend->ot->recv_cot(label, b, length);
   }
}

template<typename IO>
void eval_reveal(Backend* be, bool * b, int party, const block * label, int length) {
   SemiHonestEva<IO> * backend = (SemiHonestEva<IO>*)(be);
   block tmp;
   for (int i = 0; i < length; ++i) {
      if(isOne(&label[i]))
         b[i] = true;
      else if (isZero(&label[i]))
         b[i] = false;
      else {
         if (party == BOB or party == PUBLIC) {
            backend->io->recv_block(&tmp, 1);
            b[i] = !(block_cmp(&tmp, &label[i], 1));
         } else if (party == ALICE) {
            backend->io->send_block(&label[i], 1);
            b[i] = false;
         }
      }
   }
   if(party == PUBLIC)
      backend->io->send_data(b, length);
}

template<typename IO>
void gen_feed(Backend* be, int party, block * label, const bool*, int length);
template<typename IO>
void gen_reveal(Backend* be, bool* clear, int party, const block * label, int length);

template<typename IO>
class SemiHonestGen: public Backend { public:
    IO* io;
    SHOTIterated<IO> * ot;
    PRG prg;
    HalfGateGen<IO> * gc;
    SemiHonestGen(IO* io, HalfGateGen<IO>* gc): Backend(ALICE) {
        this->io = io;
        ot = new SHOTIterated<IO>(io, true);
        this->gc = gc;
        Feed_internal = gen_feed<IO>;
        Reveal_internal = gen_reveal<IO>;
    }
    ~SemiHonestGen() {
        delete ot;
    }
};

template<typename IO>
void gen_feed(Backend* be, int party, block * label, const bool* b, int length) {
    SemiHonestGen<IO> * backend = (SemiHonestGen<IO>*)(be);
    if(party == ALICE) {
        backend->prg.random_block(label, length);
        for (int i = 0; i < length; ++i) {
            block tosend = label[i];
            if(b[i]) tosend = xorBlocks(tosend, backend->gc->delta);
            backend->io->send_block(&tosend, 1);
        }
    } else {
        backend->ot->send_cot(label, backend->gc->delta, length);
    }
}

template<typename IO>
void gen_reveal(Backend* be, bool* b, int party, const block * label, int length) {
    SemiHonestGen<IO> * backend = (SemiHonestGen<IO>*)(be);
    for (int i = 0; i < length; ++i) {
        if(isOne(&label[i]))
            b[i] = true;
        else if (isZero(&label[i]))
            b[i] = false;
        else {
            if (party == BOB or party == PUBLIC) {
                backend->io->send_block(&label[i], 1);
                b[i] = false;
            } else if(party == ALICE) {
                block tmp;
                backend->io->recv_block(&tmp, 1);
                b[i] = !(block_cmp(&tmp, &label[i], 1));
            }
        }
    }
    if(party == PUBLIC)
        backend->io->recv_data(b, length);
}

template<typename IO>
static void setup_semi_honest(IO* io, int party) {
    if(party == ALICE) {
        local_gc = new HalfGateGen<IO>(io);
        local_backend = new SemiHonestGen<IO>(io, (HalfGateGen<IO, RTCktOpt::on>*)local_gc);
    } else {
        local_gc = new HalfGateEva<IO>(io);
        local_backend = new SemiHonestEva<IO>(io, (HalfGateEva<IO, RTCktOpt::on>*)local_gc);
    }
}

void test_millionare(int party, int number) {
    Integer a(32, number, ALICE);
    Integer b(32, number, BOB);

    cout << "ALICE Input:\t"<<a.reveal<int>()<<endl;
    cout << "BOB Input:\t"<<b.reveal<int>()<<endl;
    cout << "ALICE larger?\t"<< (a>b).reveal<bool>()<<endl;
}

int main(int argc, char *argv[]) {
    int port, party;
    parse_party_and_port(argv, &party, &port);
    NetIO * io = new NetIO(party==ALICE ? nullptr : SERVER_IP, port);

    setup_semi_honest(io, party);
    test_millionare(party, atoi(argv[3]));
    delete io;
    return 0;
}

