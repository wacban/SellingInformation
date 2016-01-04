import bitcoin_utils.*;

import org.bitcoinj.core.*;
import org.bitcoinj.params.TestNet3Params;
import org.bitcoinj.kits.WalletAppKit;
import org.bitcoinj.utils.BriefLogFormatter;
import org.bitcoinj.wallet.WalletTransaction ;
import org.bitcoinj.crypto.TransactionSignature;
import org.bitcoinj.script.Script;
import org.bitcoinj.core.Transaction.SigHash;

import org.spongycastle.crypto.params.ECDomainParameters;
import org.spongycastle.math.ec.ECPoint;
import org.spongycastle.math.ec.ECCurve;

import java.io.File;
import java.nio.ByteBuffer;

import org.apache.log4j.*;

import java.util.List;
import java.math.BigInteger;

import org.bitcoinj.core.ECKey.ECDSASignature;

class Test {
  public static void main(String argv[]) throws Exception {
    BitcoinUtilsHandler seller = new BitcoinUtilsHandler("seller");
    printBalance(seller);
    Thread.sleep(1000);
    
    BitcoinUtilsHandler buyer = new BitcoinUtilsHandler("buyer");
    printBalance(buyer);
    Thread.sleep(1000);

    BitcoinUtilsHandler test = new BitcoinUtilsHandler("test");
    printBalance(test);
    Thread.sleep(1000);
    
    // pay(test, buyer.kit.wallet().freshReceiveAddress(), 100000);
    // pay_all(seller, test.kit.wallet().freshReceiveAddress());
    // pay_all(buyer, test.kit.wallet().freshReceiveAddress());
    /*
    try {
      Thread.sleep(10000);
    } catch (Exception e) {
      System.out.println(e);
    }
    */
  }

  private static void pay(BitcoinUtilsHandler handler, Address addr, int value) {
    try{
      Coin coin = Coin.valueOf(value);
      handler.kit.wallet().sendCoins(handler.kit.peerGroup(), addr, coin);
    } catch (Exception e) {
      System.out.println(e);
    }
  }

  private static void pay_all(BitcoinUtilsHandler handler, Address addr) {
    try {
      Coin balance = handler.kit.wallet().getBalance(Wallet.BalanceType.AVAILABLE_SPENDABLE);
      Coin amount = balance.subtract(Wallet.SendRequest.DEFAULT_FEE_PER_KB.multiply(4));
      System.out.println(balance);
      System.out.println(amount);
      handler.kit.wallet().sendCoins(handler.kit.peerGroup(), addr, amount);
    } catch (Exception e) {
      System.out.println(e);
    }
  }

  private static void printBalance(BitcoinUtilsHandler handler) {
    System.out.println("--------------------------------------------");
    System.out.println("Balance:");
    System.out.println(handler.kit.wallet().getBalance());
    System.out.println("Spendable balance:");
    System.out.println(handler.kit.wallet().getBalance(Wallet.BalanceType.AVAILABLE_SPENDABLE));
    System.out.println("--------------------------------------------");
  }

  public static void main4(String argv[]) {
    BitcoinUtilsHandler seller = new BitcoinUtilsHandler("seller");
    System.out.println(seller.kit.wallet().getBalance());
    
    BitcoinUtilsHandler buyer = new BitcoinUtilsHandler("buyer");
    System.out.println(buyer.kit.wallet().getBalance());
  }

  public static void main3(String argv[]) {
    BitcoinUtilsHandler seller = new BitcoinUtilsHandler("seller");
    seller.kit.wallet().clearTransactions(0);

    BitcoinUtilsHandler buyer = new BitcoinUtilsHandler("buyer");
    buyer.kit.wallet().clearTransactions(0);

    Address addr;

    addr = seller.kit.wallet().freshReceiveAddress();
    System.out.println(addr);

    addr = buyer.kit.wallet().freshReceiveAddress();
    System.out.println(addr);
  }

  public static void main2(String argv[]) {
    init();
    try {
    BitcoinUtilsHandler seller = new BitcoinUtilsHandler("seller");
    BitcoinUtilsHandler buyer = new BitcoinUtilsHandler("buyer");

    ByteBuffer baddr = seller.freshAddress();
    Address addr = new Address(seller.kit.params(), baddr.array());

    Coin coin = Coin.valueOf(2000);
    Transaction tx = buyer.kit.wallet().createSend(addr, coin);
    
    buyer.broadcastTransaction(ByteBuffer.wrap(tx.bitcoinSerialize()));
    seller.waitForTransaction(ByteBuffer.wrap(tx.bitcoinSerialize()), 1);
    } catch (Exception e) {
      System.out.println(e);
    }
  }

  public static void main1(String argv[]) {

    BitcoinUtilsHandler handler = new BitcoinUtilsHandler("test");
    
    System.out.println(handler.kit.wallet().getBalance());
    Wallet.SendRequest.DEFAULT_FEE_PER_KB = 
    Wallet.SendRequest.DEFAULT_FEE_PER_KB.multiply(6);

    try {
      ByteBuffer baddr = handler.freshAddress();
      Address addr = new Address(handler.kit.params(), baddr.array());
      System.out.println(addr);

      Coin coin = Coin.valueOf(2000);
      Transaction tx = handler.kit.wallet().createSend(addr, coin);
      System.out.println(tx);
      handler.broadcastTransaction(ByteBuffer.wrap(tx.bitcoinSerialize()));
      handler.waitForTransaction(ByteBuffer.wrap(tx.bitcoinSerialize()), 1);
    } catch (Exception e) {
      System.out.println("Test failed!");
      System.out.println(e);
    }
  }

  public static void main0(String argv[]) {
    System.out.println("Hello world");

    BitcoinUtilsHandler handler = new BitcoinUtilsHandler("buyer");

    handler.ping();
    handler.printBalance();
   
    ECKey ec_key1 = (new ECKey()).decompress();
    System.out.println(
      handler.bytesToHex(
        ec_key1.toAddress(handler.kit.params()).getHash160()
      )
    );
    ECPoint pk1 = ec_key1.getPubKeyPoint();
    
    BigInteger ix1 = pk1.getX().toBigInteger();
    BigInteger iy1 = pk1.getY().toBigInteger();
    
    ByteBuffer x1 = ByteBuffer.wrap(ix1.toByteArray());
    ByteBuffer y1 = ByteBuffer.wrap(iy1.toByteArray());
  
    ByteBuffer t1 = handler.createSend(
      x1,
      y1,
      3000
    );
    // handler.printTransaction(t1);

    ByteBuffer addr = handler.freshAddress();
    // handler.printAddress(addr);

    ByteBuffer t2 = handler.createSendTransactionToAddress(
      t1,
      x1,
      y1,
      addr
    );

    // handler.printTransaction(t2);

    ByteBuffer bt1_output_script = handler.getOutputScript(t1, x1, y1);
    Script t1_output_script = new Script(bt1_output_script.array());
    System.out.println(t1_output_script);

    ByteBuffer bt2_hash = handler.hashForSignature(
      t2, 
      x1, 
      y1, 
      bt1_output_script
    );

    Sha256Hash t2_hash = new Sha256Hash(bt2_hash.array());
    ECDSASignature sig = ec_key1.sign(t2_hash);
    
    ByteBuffer r = ByteBuffer.wrap(sig.r.toByteArray());
    ByteBuffer s = ByteBuffer.wrap(sig.s.toByteArray());

    ByteBuffer res = handler.getSignedTransaction(
      t2,
      x1,
      y1,
      r,
      s,
      bt1_output_script
    );
    if (res.array().length == 0) {
      System.out.println("getSignedTransaction failed");
    }
  }

  private static void init(){
    Wallet.SendRequest.DEFAULT_FEE_PER_KB = Coin.valueOf(6000);
  }
}
