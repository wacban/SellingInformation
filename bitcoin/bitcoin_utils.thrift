namespace cpp bitcoin_utils
namespace java bitcoin_utils

typedef binary Address
typedef binary Transaction
typedef binary IntegerBytes
typedef binary Signature
typedef binary Script
typedef binary Hash

struct IntegerPair {
  1: required IntegerBytes r;
  2: required IntegerBytes s;
}

service BitcoinUtils { 
   void ping(),
   void sleep(),
   
   /**
    * Returns a fresh address for the wallet.
    */
   Address freshAddress(),

   /**
    * Creates transaction that sends value satoshis to 
    * address x, y.
    */
   Transaction createSend(1:IntegerBytes x, 2:IntegerBytes y, 3:i32 value),
   
   /**
    * Creates transaction that spends P2PKH output of transaction tx.
    * The output is for public key x, y. Output of this transaction is
    * to address dst.
    */
   Transaction createSendTransactionToAddress(
     1:Transaction tx, 
     2:IntegerBytes x, 
     3:IntegerBytes y, 
     4:Address dst
   ),

   Script getOutputScript(
     1:Transaction tx,
     2:IntegerBytes x, 
     3:IntegerBytes y,
   ),

   Hash hashForSignature(
     1:Transaction tx
     2:IntegerBytes x, 
     3:IntegerBytes y,
     4:Script redeem_script
   ),

   Transaction getSignedTransaction(
     1:Transaction tx, 
     2:IntegerBytes x, 
     3:IntegerBytes y,
     4:IntegerBytes r,
     5:IntegerBytes s,
     6:Script redeem_script
   ),
/*
   bool verifySignature(
     1:Transaction tx, 
     2:IntegerBytes x, 
     3:IntegerBytes y,
     4:IntegerBytes r,
     5:IntegerBytes s,
     6:Script redeem_script
   ),
 */

   bool cppVerifySignature(
     1:binary data,
     2:IntegerBytes x, 
     3:IntegerBytes y,
     4:IntegerBytes r,
     5:IntegerBytes s
   ),

   bool broadcastTransaction(
     1:Transaction tx
   ),

   bool verifyTransaction(
     1:Transaction tx,
     2:IntegerBytes x, 
     3:IntegerBytes y,
     4:i32 value
   ),

   bool waitForTransaction(
     1:Transaction tx,
     2:i32 depth,
   ),

   bool waitForTransactionByOutput(
     1:Address addr,
     2:i32 depth,
   ),

   IntegerPair getSignature(
     1:Address addr,
   ),

   bool redeemTransaction(
     1:Transaction tx,
     2:IntegerBytes sk,
     3:IntegerBytes x, 
     4:IntegerBytes y,
   ),
}
