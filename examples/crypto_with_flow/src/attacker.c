#include "../annotated_api/polarssl_definitions/polarssl_definitions.h"

#include <stdlib.h>
#include <string.h>
//@ #include <quantifiers.gh>

#define POLARSSL_ATTACKER_ITERATIONS 100

/*@

predicate_ctor attacker_random_pred(predicate(cryptogram) pub,
                                    predicate() proof_pred)() =
  is_bad_random_is_public(_, pub, proof_pred) &*&
  proof_pred()
;

lemma void close_havege_util(predicate(cryptogram) pub, 
                             predicate() pred, int principal)
  requires is_principal_with_public_random(_, pub,
                                           attacker_random_pred(pub, pred), 
                                           principal) &*& 
           is_bad_random_is_public(_, pub, pred) &*& pred();
  ensures  havege_util(pub, attacker_random_pred(pub, pred), principal);
{
  close attacker_random_pred(pub, pred)();
  close havege_util(pub, attacker_random_pred(pub, pred), principal);
}

lemma void open_havege_util(predicate(cryptogram) pub, 
                            predicate() pred, int principal)
  requires havege_util(pub, attacker_random_pred(pub, pred), principal);
  ensures  is_principal_with_public_random(_, pub,
                                           attacker_random_pred(pub, pred), 
                                           principal) &*&
           is_bad_random_is_public(_, pub, pred) &*&
           pred();
{
  open havege_util(pub, attacker_random_pred(pub, pred), principal);
  open attacker_random_pred(pub, pred)();
}

predicate attacker_invariant(predicate(cryptogram) pub,
                             predicate() pred,
                             struct havege_state* state,
                             void* socket, int attacker) =
  [_]public_invar(pub) &*& pred() &*&
  havege_state_initialized(state) &*&
  integer(socket, ?fd) &*& net_status(fd, ?ip, ?port, connected) &*&
  true == bad(attacker) &*& principal(attacker) &*& 
  generated_values(attacker, ?count) &*&
  is_principal_with_public_random(_, pub, 
                                  attacker_random_pred(pub, pred), 
                                  attacker) &*&
    is_bad_random_is_public(_, pub, pred) &*&
    is_bad_key_is_public(_, pub, pred) &*&
    is_public_key_is_public(_, pub, pred) &*&
    is_bad_private_key_is_public(_, pub, pred) &*&
    is_hash_is_public(_, pub, pred) &*&
    is_public_hmac_is_public(_, pub, pred) &*&
    is_public_encryption_is_public(_, pub, pred) &*&
    is_public_decryption_is_public(_, pub, pred) &*&
    is_public_auth_encryption_is_public(_, pub, pred) &*&
    is_public_auth_decryption_is_public(_, pub, pred) &*&
    is_public_asym_encryption_is_public(_, pub, pred) &*&
    is_public_asym_decryption_is_public(_, pub, pred) &*&
    is_public_asym_signature_is_public(_, pub, pred)
;

@*/

void attacker_send_data(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size;
  char buffer[MAX_MESSAGE_SIZE];
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ close_havege_util(pub, pred, attacker);
  r_int_with_bounds(havege_state, &temp, 1, MAX_MESSAGE_SIZE);
  //@ open_havege_util(pub, pred, attacker);
  size = temp;
  //@ chars_split(buffer, size);
  net_send(socket, buffer, (unsigned int) size);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
}

void attacker_send_concatenation(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int size1;
  int size2;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];

  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 <= 0 || MAX_MESSAGE_SIZE - size1 <= size2)
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return; 
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);
  
  //@ close optional_crypto_chars(false, buffer1, size1, cs1);
  memcpy(buffer3, buffer1, (unsigned int) size1);
  //@ open optional_crypto_chars(false, buffer1, size1, cs1);
  //@ open optional_crypto_chars(false, buffer3, size1, cs1);

  //@ close optional_crypto_chars(false, buffer2, size2, cs2);
  memcpy((char*) buffer3 + size1, buffer2, (unsigned int) size2);
  //@ open optional_crypto_chars(false, buffer2, size2, cs2);
  //@ open optional_crypto_chars(false, (void*) buffer3 + size1, size2, cs2);

  net_send(socket, buffer3, (unsigned int) (size1 + size2));
  
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
}

void attacker_send_split(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  char buffer[MAX_MESSAGE_SIZE];
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  size1 = net_recv(socket, buffer, MAX_MESSAGE_SIZE);
  if (size1 <= 0)
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer, size1, ?cs);

  //@ close_havege_util(pub, pred, attacker);
  r_int_with_bounds(havege_state, &temp, 0, size1);
  //@ open_havege_util(pub, pred, attacker);
  size2 = temp;
  net_send(socket, buffer, (unsigned int) (size2));
  net_send(socket, (void*) buffer + size2, 
            (unsigned int) (size1 - size2));

  //@ chars_join(buffer);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
}

void attacker_send_random(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size;
  char buffer[MAX_MESSAGE_SIZE];

  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ close_havege_util(pub, pred, attacker);
  r_int_with_bounds(havege_state, &temp, MIN_RANDOM_SIZE, MAX_MESSAGE_SIZE);
  size = temp;
  r_int_with_bounds(havege_state, &temp, 0, INT_MAX);
  //@ open_havege_util(pub, pred, attacker);
  //@ close random_request(attacker, temp, false);
  if (havege_random(havege_state, buffer, (unsigned int) size) == 0)
  {
    //@ assert cryptogram(buffer, size, ?cs, ?cg);
    //@ assert is_bad_random_is_public(?proof, pub, pred);
    //@ proof(cg);
    //@ public_cryptogram(buffer, cg);
    net_send(socket, buffer, (unsigned int) size);
  }
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
}

int attacker_key_item_havege_random_stub(void *havege_state,
                                         char *output, size_t len)
  //@ requires PRG_PRECONDITION(havege_state_initialized, havege_state);
  //@ ensures PRG_POSTCONDITION(havege_state_initialized, havege_state);
{
  return havege_random(havege_state, output, len);
}

void attacker_send_keys(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  pk_context context;
  pk_context context_pub;
  pk_context context_priv;
  unsigned int key_size;
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  unsigned int temp;
  //@ close_havege_util(pub, pred, attacker);
  r_u_int_with_bounds(havege_state, &temp, 1024, 8192);
  //@ open_havege_util(pub, pred, attacker);
  key_size = temp;
  char* key = malloc((int) key_size);
  if ((key) == 0) abort();
  char* pub_key = malloc((int) key_size);
  if ((pub_key) == 0) abort();
  char* priv_key = malloc((int) key_size);
  if ((priv_key) == 0) abort();
  
  //@ close random_request(attacker, temp, true);
  if (havege_random(havege_state, key, key_size) != 0) abort();
  
  //@ close pk_context(&context);
  pk_init(&context);
  //@ close pk_context(&context_pub);
  pk_init(&context_pub);
  //@ close pk_context(&context_priv);
  pk_init(&context_priv);
  
  if (pk_init_ctx(&context, pk_info_from_type(POLARSSL_PK_RSA)) != 0) 
    abort();
  //@ close rsa_key_request(attacker, 0);
  //@ close random_state_predicate(havege_state_initialized);
  /*@ produce_function_pointer_chunk random_function(
                      attacker_key_item_havege_random_stub)
                     (havege_state_initialized)(state, out, len) { call(); } @*/
  if (rsa_gen_key(context.pk_ctx, attacker_key_item_havege_random_stub, 
                  havege_state, key_size, 65537) != 0) abort();
  
  if (pk_write_pubkey_pem(&context, pub_key, key_size) != 0) abort();
  if (pk_write_key_pem(&context, priv_key, key_size) != 0) abort();
  if (pk_parse_public_key(&context_pub, pub_key, key_size) != 0) abort();
  if (pk_parse_key(&context_priv, priv_key, key_size, NULL, 0) != 0) abort();
  
  //@ assert is_bad_key_is_public(?proof1, pub, pred);
  //@ assert cryptogram(key, key_size, ?key_cs, ?key_cg);
  //@ proof1(key_cg);
  //@ public_cryptogram(key, key_cg);
  net_send(socket, key, key_size);
    
  //@ assert is_public_key_is_public(?proof2, pub, pred);
  //@ assert cryptogram(pub_key, key_size, ?pub_key_cs, ?pub_key_cg);
  //@ proof2(pub_key_cg);
  //@ public_cryptogram(pub_key, pub_key_cg);
  net_send(socket, pub_key, key_size);
  
  //@ assert is_bad_private_key_is_public(?proof3, pub, pred);
  //@ assert cryptogram(priv_key, key_size, ?priv_key_cs, ?priv_key_cg);
  //@ proof3(priv_key_cg);
  //@ public_cryptogram(priv_key, priv_key_cg);
  net_send(socket, priv_key, key_size);
  
  //@ open random_state_predicate(havege_state_initialized);
  //@ pk_release_context_with_keys(&context);
  pk_free(&context);
  //@ open pk_context(&context);
  //@ pk_release_context_with_key(&context_pub);
  pk_free(&context_pub);
  //@ open pk_context(&context_pub);
  //@ pk_release_context_with_key(&context_priv);
  pk_free(&context_priv);
  //@ open pk_context(&context_priv);
  free(key);
  free(pub_key);
  free(priv_key);
  
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
}

void attacker_send_hash(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size;
  char buffer[MAX_MESSAGE_SIZE];

  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);

  size = net_recv(socket, buffer, MAX_MESSAGE_SIZE);
  if (size < MIN_HMAC_INPUT_SIZE)
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer, size, ?pay);

  char hash[64];
  //@ close optional_crypto_chars(false, buffer, size, pay);
  sha512(buffer, (unsigned int) size, hash, 0);
  //@ open optional_crypto_chars(false, buffer, size, pay);
  //@ assert cryptogram(hash, 64, ?h_cs, ?h_cg);
  //@ assert h_cg == cg_hash(pay);
  //@ assert is_hash_is_public(?proof, pub, pred);
  //@ public_chars(buffer, size, pay);
  //@ proof(h_cg);
  //@ public_cryptogram(hash, h_cg);
  net_send(socket, hash, 64);
  
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
}

void attacker_send_hmac(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[64];

  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_HMAC_INPUT_SIZE)
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);

  //@ interpret_symmetric_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, cg_symmetric_key(?p, ?c));
  //@ close optional_crypto_chars(false, buffer2, size2, cs2);
  sha512_hmac(buffer1, (unsigned int) size1, buffer2, 
              (unsigned int) size2, buffer3, 0);
  //@ assert cryptogram(buffer3, 64, ?cs_hmac, ?hmac);
  //@ assert is_public_hmac_is_public(?proof2, pub, pred);
  //@ public_chars(buffer2, size2, cs2);
  //@ proof2(hmac);
  //@ public_cryptogram(buffer3, hmac);
  net_send(socket, buffer3, 64);
  //@ public_cryptogram(buffer1, cg_symmetric_key(p, c));
  
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
}

void attacker_send_encrypted(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];
  aes_context aes_context;
  size_t iv_off;
  char iv[16];
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_ENC_SIZE || 
      (size1 != 16 && size1 != 24 && size1 != 32))
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);
  
  //@ close_havege_util(pub, pred, attacker);
  r_u_int_with_bounds(havege_state, &iv_off, 0, 15);
  //@ open_havege_util(pub, pred, attacker);
  //@ assert u_integer(&iv_off, ?iv_off_val);
  
  //@ close aes_context(&aes_context);
  //@ interpret_symmetric_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, ?cg_key);
  //@ assert cg_key == cg_symmetric_key(?p, ?c);
  if (aes_setkey_enc(&aes_context, buffer1, (unsigned int) size1 * 8) == 0)
  {
    //@ close random_request(attacker, 0, false);
    if (havege_random(havege_state, iv, 16) == 0)
    {
      //@ assert cryptogram(iv, 16, ?cs_iv, ?cg_iv);
      /*@ assert is_principal_with_public_random(?proof1, pub,
                                                 attacker_random_pred(pub, pred),
                                                 attacker); @*/
      //@ close attacker_random_pred(pub, pred)();
      //@ proof1(cg_iv);
      //@ open attacker_random_pred(pub, pred)();
      //@ public_cryptogram(iv, cg_iv);
      //@ close optional_crypto_chars(false, buffer2, size2, cs2); 
      if (aes_crypt_cfb128(&aes_context, AES_ENCRYPT, 
                            (unsigned int) size2, &iv_off, iv, buffer2, 
                            buffer3) == 0)
      {
        /*@ 
          {
            assert cryptogram(buffer3, size2, ?cs_enc, ?cg_enc);
            assert cg_enc == cg_encrypted(p, c, cs2,
                               append(chars_of_int(iv_off_val), cs_iv));
            assert [_]pub(cg_key);
            assert is_public_encryption_is_public(?proof2, pub, pred);
            public_chars(buffer2, size2, cs2);
            proof2(cg_enc);
            public_cryptogram(buffer3, cg_enc);
          }
        @*/
        net_send(socket, buffer3, (unsigned int) size2);
      }
      //@ open optional_crypto_chars(false, buffer2, size2, cs2); 
    }
    aes_free(&aes_context);
    //@ open aes_context(&aes_context);
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    //@ public_cryptogram(buffer1, cg_symmetric_key(p, c));
    return;
  }
  
  //@ open aes_context(&aes_context);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ public_cryptogram(buffer1, cg_symmetric_key(p, c));
  return;
}

void attacker_send_decrypted(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];
  aes_context aes_context;
  char iv[16];
  size_t iv_off;
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_ENC_SIZE ||
       (size1 != 16 && size1 != 24 && size1 != 32))
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);

  //@ close aes_context(&aes_context);
  //@ interpret_symmetric_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, ?cg_key);
  //@ assert cg_key == cg_symmetric_key(?p, ?c);
  if (aes_setkey_enc(&aes_context, buffer1, (unsigned int) size1 * 8) == 0)
  {
    
    //@ close_havege_util(pub, pred, attacker);
    r_u_int_with_bounds(havege_state, &iv_off, 0, 15);
    //@ open_havege_util(pub, pred, attacker);
    //@ assert u_integer(&iv_off, ?iv_off_val);
    //@ close random_request(attacker, 0, false);
    if (havege_random(havege_state, iv, 16) == 0)
    {
      //@ assert cryptogram(iv, 16, ?cs_iv, ?cg_iv);
      /*@ assert is_principal_with_public_random(?proof1, pub,
                                  attacker_random_pred(pub, pred), 
                                  attacker); @*/
      //@ close attacker_random_pred(pub, pred)();
      //@ proof1(cg_iv);
      //@ open attacker_random_pred(pub, pred)();
      //@ public_cryptogram(iv, cg_iv);
      //@ close optional_crypto_chars(false, buffer2, size2, cs2);
      if (aes_crypt_cfb128(&aes_context, AES_DECRYPT, 
                            (unsigned int) size2, &iv_off, iv, buffer2, 
                            buffer3) == 0)
      {
        /*@ 
          {
            assert crypto_chars(buffer3, size2, ?cs_output);
            cryptogram cg_enc = cg_encrypted(p, c, cs_output,
                                  append(chars_of_int(iv_off_val), cs_iv));
            assert cs2 == chars_for_cg(cg_enc);
            open optional_crypto_chars(false, buffer2, size2, cs2);
            public_chars_extract(buffer2, cg_enc);
            assert [_]pub(cg_enc);
            assert is_public_decryption_is_public(?proof2, pub, pred);
            proof2(cg_key, cg_enc);
            public_crypto_chars(buffer3, size2, cs_output);
          }
        @*/
        net_send(socket, buffer3, (unsigned int) size2);
      }
      //@ open optional_crypto_chars(false, buffer2, size2, cs2);
    }
    aes_free(&aes_context);
  }
  //@ open aes_context(&aes_context);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ public_cryptogram(buffer1, cg_key);
}

void attacker_send_auth_encrypted(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];
  gcm_context gcm_context;
  char iv[16];
  char mac[16];
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_ENC_SIZE ||
      (size1 != 16 && size1 != 24 && size1 != 32))
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);

  //@ close gcm_context(&gcm_context);
  //@ interpret_symmetric_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, ?cg_key);
  //@ assert cg_key == cg_symmetric_key(?p, ?c);
  if (gcm_init(&gcm_context, POLARSSL_CIPHER_ID_AES, 
      buffer1, (unsigned int) size1 * 8) == 0)
  {
    //@ close random_request(attacker, 0, false);
    if (havege_random(havege_state, iv, 16) == 0)
    {
      
      //@ assert cryptogram(iv, 16, ?cs_iv, ?cg_iv);
      /*@ assert is_principal_with_public_random(?proof1, pub,
                                  attacker_random_pred(pub, pred), 
                                  attacker); @*/
      //@ close attacker_random_pred(pub, pred)();
      //@ proof1(cg_iv);
      //@ open attacker_random_pred(pub, pred)();
      //@ public_cryptogram(iv, cg_iv);
      //@ public_chars(iv, 16, cs_iv);
      //@ close optional_crypto_chars(false, buffer2, size2, cs2);
      if (gcm_crypt_and_tag(&gcm_context, GCM_ENCRYPT, 
                            (unsigned int) size2, iv, 16, NULL, 0, buffer2, 
                            buffer3, 16, mac) == 0)
      {
        /*@ 
          {
            assert cryptogram(buffer3, size2, ?cs_enc, ?cg_enc);
            public_chars(buffer2, size2, cs2);
            assert is_public_auth_encryption_is_public(?proof2, pub, pred);
            proof2(cg_enc);
            public_cryptogram(buffer3, cg_enc);
          }
        @*/
        net_send(socket, buffer3, (unsigned int) size2);
      }
      //@ open optional_crypto_chars(false, buffer2, size2, cs2);
    }
    gcm_free(&gcm_context);
  }
  //@ open gcm_context(&gcm_context);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ public_cryptogram(buffer1, cg_key);
}

void attacker_send_auth_decrypted(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];
  gcm_context gcm_context;
  char iv[16];
  char tag[16];
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_ENC_SIZE ||
      (size1 != 16 && size1 != 24 && size1 != 32))
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);
  
  //@ close gcm_context(&gcm_context);
  //@ interpret_symmetric_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, ?cg_key);
  //@ assert cg_key == cg_symmetric_key(?p, ?c);
  if (gcm_init(&gcm_context, POLARSSL_CIPHER_ID_AES, 
      buffer1, (unsigned int) size1 * 8) == 0)
  {
    //@ close random_request(attacker, 0, false);
    if (havege_random(havege_state, iv, 16) == 0)
    {
      //@ assert cryptogram(iv, 16, ?cs_iv, ?cg_iv);
      /*@ assert is_principal_with_public_random(?proof1, pub,
                                  attacker_random_pred(pub, pred), 
                                  attacker); @*/
      //@ close attacker_random_pred(pub, pred)();
      //@ proof1(cg_iv);
      //@ open attacker_random_pred(pub, pred)();
      //@ public_cryptogram(iv, cg_iv);
      //@ public_chars(iv, 16, cs_iv);
      //@ close random_request(attacker, 0, false);
      if (havege_random(havege_state, tag, 16) == 0)
      {
        //@ assert cryptogram(tag, 16, ?cs_mac, ?cg_mac);
        /*@ assert is_principal_with_public_random(?proof2, pub,
                                    attacker_random_pred(pub, pred), 
                                    attacker); @*/
        //@ close attacker_random_pred(pub, pred)();
        //@ proof2(cg_mac);
        //@ open attacker_random_pred(pub, pred)();
        //@ public_cryptogram(tag, cg_mac);
        //@ public_chars(tag, 16, cs_mac);
        //@ close optional_crypto_chars(false, buffer2, size2, cs2);
        if (gcm_auth_decrypt(&gcm_context, (unsigned int) size2,
                              iv, 16, NULL, 0, tag, 16,
                              buffer2, buffer3) == 0)
        {
          /*@
            {
              assert crypto_chars(buffer3, size2, ?cs_output);
              cryptogram cg_enc =
                           cg_auth_encrypted(p, c, cs_mac, cs_output, cs_iv);
              assert cs2 == chars_for_cg(cg_enc);
              public_chars(buffer2, size2, cs2);
              public_chars_extract(buffer2, cg_enc);
              assert [_]pub(cg_enc);
              assert is_public_auth_decryption_is_public(?proof3, pub, pred);
              proof3(cg_key, cg_enc);
              public_crypto_chars(buffer3, size2, cs_output);
            }
          @*/
          net_send(socket, buffer3, (unsigned int) size2);
        }
        //@ open optional_crypto_chars(false, buffer2, size2, cs2);
      }
    }
    gcm_free(&gcm_context);
  }
  //@ open gcm_context(&gcm_context);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ public_cryptogram(buffer1, cg_key);
}

void attacker_send_asym_encrypted(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  unsigned int osize;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];
  pk_context context;
  
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_ENC_SIZE)
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);
  
  //@ close pk_context(&context);
  pk_init(&context);
  //@ interpret_public_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, ?cg_key);
  //@ assert cg_key == cg_public_key(?p, ?c);
  if (pk_parse_public_key(&context, buffer1, (unsigned int) size1) == 0)
  {
    if (size2 * 8 <= size1)
    {
      //@ close random_state_predicate(havege_state_initialized);
      /*@ produce_function_pointer_chunk random_function(
                    attacker_key_item_havege_random_stub)
                  (havege_state_initialized)(state, out, len) { call(); } @*/
      //@ close optional_crypto_chars(false, buffer2, size2, cs2);
      if (pk_encrypt(&context, buffer2, (unsigned int) size2, 
                    buffer3, &osize, MAX_MESSAGE_SIZE,
                    attacker_key_item_havege_random_stub, havege_state) == 0)
      {        
        //@ assert u_integer(&osize, ?osize_val);
        /*@ 
          {
            assert cryptogram(buffer3, osize_val, ?cs_enc, ?cg_enc);
            assert cg_enc == cg_asym_encrypted(p, c, cs2, _);
            assert is_public_asym_encryption_is_public(?proof, pub, pred);
            public_chars(buffer2, size2, cs2);
            proof(cg_enc);
            assert [_]pub(cg_enc);
            public_cryptogram(buffer3, cg_enc);
          }
        @*/
        net_send(socket, buffer3, osize);
      }
      //@ open optional_crypto_chars(false, buffer2, size2, cs2);
    }
    //@ pk_release_context_with_key(&context);
  }
  pk_free(&context);
  //@ open pk_context(&context);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ public_cryptogram(buffer1, cg_key);
}

void attacker_send_asym_decrypted(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  unsigned int osize;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];
  pk_context context;

  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_ENC_SIZE)
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);
  
  //@ close pk_context(&context);
  pk_init(&context);
  //@ interpret_private_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, ?cg_key);
  //@ assert cg_key == cg_private_key(?p, ?c);
  if (pk_parse_key(&context, buffer1, (unsigned int) size1, NULL, 0) == 0)
  {
    if (size2 * 8 <= size1)
    {
      //@ close random_state_predicate(havege_state_initialized);
      /*@ produce_function_pointer_chunk random_function(
                    attacker_key_item_havege_random_stub)
                  (havege_state_initialized)(state, out, len) { call(); } @*/
      //@ close optional_crypto_chars(false, buffer2, size2, cs2);
      if (pk_decrypt(&context, buffer2, (unsigned int) size2, buffer3, &osize,
                    MAX_MESSAGE_SIZE, 
                    attacker_key_item_havege_random_stub, havege_state) == 0)
      {
        //@ assert u_integer(&osize, ?osize_val);
        /*@
          {
            assert crypto_chars(buffer3, osize_val, ?cs_output);
            assert [_]exists(?ent);
            cryptogram cg_enc = cg_asym_encrypted(p, c, cs_output, ent);
            public_chars(buffer2, size2, cs2);
            public_chars_extract(buffer2, cg_enc);
            assert [_]pub(cg_enc);
            assert is_public_asym_decryption_is_public(?proof, pub, pred);
            proof(cg_key, cg_enc);
            public_crypto_chars(buffer3, osize_val, cs_output);
          }
        @*/
        net_send(socket, buffer3, osize);
      }
      //@ open optional_crypto_chars(false, buffer2, size2, cs2);
    }
    //@ pk_release_context_with_key(&context);
  }
  pk_free(&context);
  //@ open pk_context(&context);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ public_cryptogram(buffer1, cg_key);
}

void attacker_send_asym_signature(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int temp;
  int size1;
  int size2;
  unsigned int osize;
  char buffer1[MAX_MESSAGE_SIZE];
  char buffer2[MAX_MESSAGE_SIZE];
  char buffer3[MAX_MESSAGE_SIZE];
  pk_context context;

  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  
  size1 = net_recv(socket, buffer1, MAX_MESSAGE_SIZE);
  size2 = net_recv(socket, buffer2, MAX_MESSAGE_SIZE);
  if (size1 <= 0 || size2 < MIN_ENC_SIZE)
  {
    //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
    return;
  }
  //@ assert chars(buffer1, size1, ?cs1);
  //@ assert chars(buffer2, size2, ?cs2);
  
  //@ close pk_context(&context);
  pk_init(&context);
  //@ interpret_private_key(buffer1, size1, cs1);
  //@ assert cryptogram(buffer1, size1, cs1, ?cg_key);
  //@ assert cg_key == cg_private_key(?p, ?c);
  if (pk_parse_key(&context, buffer1, (unsigned int) size1, NULL, 0) == 0)
  {
    if (size2 * 8 < size1)
    {
      //@ close random_state_predicate(havege_state_initialized);
      /*@ produce_function_pointer_chunk random_function(
                  attacker_key_item_havege_random_stub)
                  (havege_state_initialized)(state, out, len) { call(); } @*/
      //@ close optional_crypto_chars(false, buffer2, size2, cs2);
      if (pk_sign(&context, POLARSSL_MD_NONE, buffer2, (unsigned int) size2, 
                  buffer3, &osize, attacker_key_item_havege_random_stub, 
                  havege_state) == 0)
      {
        /*@
          {
            assert u_integer(&osize, ?osize_val);
            assert cryptogram(buffer3, osize_val, ?cs_enc, ?cg_sig);
            assert cg_sig == cg_asym_signature(p, c, cs2, _);
            assert is_public_asym_signature_is_public(?proof, pub, pred);
            public_chars(buffer2, size2, cs2);
            proof(cg_sig);
            public_cryptogram(buffer3, cg_sig);
          }
        @*/
        net_send(socket, buffer3, osize);
      }
      //@ open optional_crypto_chars(false, buffer2, size2, cs2);
    }
    //@ pk_release_context_with_key(&context);
  }
  pk_free(&context);
  //@ open pk_context(&context);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ public_cryptogram(buffer1, cg_key);
}

void attacker_core(havege_state *havege_state, void* socket)
  //@ requires attacker_invariant(?pub, ?pred, havege_state, socket, ?attacker);
  //@ ensures  attacker_invariant(pub, pred, havege_state, socket, attacker);
{
  int action;
  //@ open attacker_invariant(pub, pred, havege_state, socket, attacker);
  //@ close_havege_util(pub, pred, attacker);
  r_int(havege_state, &action);
  //@ open_havege_util(pub, pred, attacker);
  //@ close attacker_invariant(pub, pred, havege_state, socket, attacker);

  switch (action % 14)
  {
    case 0:
      attacker_send_data(havege_state, socket);
      break;
    case 1:
      attacker_send_concatenation(havege_state, socket);
      break;
    case 2:
      attacker_send_split(havege_state, socket);
      break;
    case 3:
      attacker_send_random(havege_state, socket);
      break;
    case 4:
      attacker_send_keys(havege_state, socket);
      break;
    case 5:
      attacker_send_hash(havege_state, socket);
      break;
    case 6:
      attacker_send_hmac(havege_state, socket);
      break;
    case 7:
      attacker_send_encrypted(havege_state, socket);
      break;
    case 8:
      attacker_send_decrypted(havege_state, socket);
      break;
    case 9:
      attacker_send_auth_encrypted(havege_state, socket);
      break;
    case 10:
      attacker_send_auth_decrypted(havege_state, socket);
      break;
    case 11:
      attacker_send_asym_encrypted(havege_state, socket);
      break;
    case 12:
      attacker_send_asym_decrypted(havege_state, socket);
      break;
    case 13:
      attacker_send_asym_signature(havege_state, socket);
      break;
  }
}

void attacker()
  /*@ requires [_]public_invar(?pub) &*&
               public_invariant_constraints(pub, ?proof_pred) &*& 
               proof_pred() &*& 
               principals(?count1); @*/
  /*@ ensures  public_invariant_constraints(pub, proof_pred) &*& 
               proof_pred() &*&
               principals(?count2) &*& count2 > count1; @*/
{
  bool havege_failure = false;
  int server_or_client;
  int port;
  int* socket;
  int socket1;
  int socket2;
  //@ int bad_one = principal_create();
  //@ assume (bad(bad_one));
  
  havege_state havege_state;
  //@ close havege_state(&havege_state);
  havege_init(&havege_state);
  
  //@ open public_invariant_constraints(pub, proof_pred);
  /*@
  {
    lemma void principal_with_public_random(cryptogram random)
      requires  is_bad_random_is_public(?proof, pub, proof_pred) &*&
                proof_pred() &*&
                random == cg_random(bad_one, _);
      ensures   is_bad_random_is_public(proof, pub, proof_pred) &*&
                proof_pred() &*&
                [_]pub(random);
    {
      proof(random);
    }
    produce_lemma_function_pointer_chunk(principal_with_public_random) :
      principal_with_public_random
        (pub, attacker_random_pred(pub, proof_pred), bad_one)(random__) 
        { 
          open attacker_random_pred(pub, proof_pred)();
          call(); 
          close attacker_random_pred(pub, proof_pred)();
        }
    {duplicate_lemma_function_pointer_chunk(principal_with_public_random);};
  }
  @*/
  //@ close_havege_util(pub, proof_pred, bad_one);
  r_int(&havege_state, &server_or_client);
  r_int(&havege_state, &port);
  //@ open_havege_util(pub, proof_pred, bad_one);
  
  bool network_failure = false;
  
  if (server_or_client % 2 == 0)
  {
    if(net_connect(&socket1, NULL, port) != 0) network_failure = true;
    else if(net_set_block(socket1) != 0) network_failure = true;
    socket = &socket1;
  }
  else
  {
    if(net_bind(&socket1, NULL, port) != 0) 
      {network_failure = true;}
    else if(net_accept(socket1, &socket2, NULL) != 0)
      {net_close(socket1); network_failure = true;}
    else if(net_set_block(socket2) != 0) 
      {net_close(socket1); network_failure = true;}
    socket = &socket2;
  }

  if (!network_failure)
  {
    //@ close attacker_invariant(pub, proof_pred, &havege_state, socket, bad_one);
    int j = 0;
    while(j < POLARSSL_ATTACKER_ITERATIONS)
      /*@ invariant attacker_invariant(pub, proof_pred, 
                                       &havege_state, socket, bad_one); @*/
    {
      attacker_core(&havege_state, socket);
      j++;
    }
    //@ open attacker_invariant(pub, proof_pred, &havege_state, socket, bad_one);

    if (server_or_client % 2 == 0)
      net_close(socket1);
    else
    {
      net_close(socket1);
      net_close(socket2);
    }
  }
  
  //@ close public_invariant_constraints(pub, proof_pred);
  //@ principal_destroy(bad_one);
  havege_free(&havege_state);
  //@ open havege_state(&havege_state);
  /*@ leak is_principal_with_public_random(_, pub, 
                            attacker_random_pred(pub, proof_pred), bad_one); @*/
}
