import bitcoin_utils.*;

import org.apache.thrift.TException;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;

import org.bitcoinj.core.*;
import org.bitcoinj.params.TestNet3Params;
import org.bitcoinj.kits.WalletAppKit;
import org.bitcoinj.utils.BriefLogFormatter;
import org.bitcoinj.wallet.WalletTransaction ;
import org.bitcoinj.crypto.TransactionSignature;
import org.bitcoinj.script.Script;
import org.bitcoinj.core.Transaction.SigHash;
import org.bitcoinj.core.ECKey.ECDSASignature;

import org.spongycastle.crypto.params.ECDomainParameters;
import org.spongycastle.math.ec.ECPoint;
import org.spongycastle.math.ec.ECCurve;

import java.io.File;
import java.nio.ByteBuffer;

import org.apache.log4j.*;

import java.util.List;
import java.math.BigInteger;

import java.lang.Thread;

class Ping extends Thread {
  BitcoinUtils.Client client;

  public Ping() {
    try {
    int port = 9091;
    System.out.println("Port: " + port);
    TTransport transport;
    transport = new TSocket("localhost", port);
    transport.open();

    TProtocol protocol = new  TBinaryProtocol(transport);
    client = new BitcoinUtils.Client(protocol);
    } catch (Exception e) {
      System.out.println(e);
    }
  }

  public void run(){
    try {
      System.out.println("sleep start");
      client.sleep();
      System.out.println("sleep end");
    } catch (Exception e) {
      System.out.println(e);
    }
  }
}

public class JavaClient {
  public static void main(String [] args) {
    try {
      /*
      int port = 9091;
      System.out.println("Port: " + port);
      TTransport transport;
      transport = new TSocket("localhost", port);
      transport.open();

      TProtocol protocol = new  TBinaryProtocol(transport);
      BitcoinUtils.Client client = new BitcoinUtils.Client(protocol);
      */

      Thread[] t = new Thread[10];
      
      for(int i = 0; i < 10; ++i) {
        t[i] = new Ping();
      }

      for(int i = 0; i < 10; ++i) {
        t[i].start();
      }
      
      for(int i = 0; i < 10; ++i) {
        t[i].join();
      }

      // transport.close();
    } catch (Exception e) {
      System.out.println(e);
    } 
  }

  private static void perform(BitcoinUtils.Client client) throws TException
  {
    try{
    client.ping();
    System.out.println("ping()");

    BitcoinUtilsHandler buyer = new BitcoinUtilsHandler("buyer");

    ByteBuffer baddr = buyer.freshAddress();
    Address addr = new Address(buyer.kit.params(), baddr.array());
    Coin coin = Coin.valueOf(2000);
    Transaction tx = buyer.kit.wallet().createSend(addr, coin);

    buyer.broadcastTransaction(ByteBuffer.wrap(tx.bitcoinSerialize()));
    client.waitForTransaction(ByteBuffer.wrap(tx.bitcoinSerialize()), 1);
    } catch (Exception e) {
      System.out.println(e);
    }
  }

  private static void perform0(BitcoinUtils.Client client) throws TException
  {
    client.ping();
    System.out.println("ping()");

    BitcoinUtilsHandler handler = new BitcoinUtilsHandler("buyer");

    client.ping();
    System.out.println("ping()");

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

    System.out.println(ix1);
    System.out.println(iy1);
    System.out.println(sig.r);
    System.out.println(sig.s);

    boolean res = client.cppVerifySignature(
      bt2_hash,
      x1,
      y1,
      r,
      s
    );

/*
    res = handler.verifySignature(
      t2,
      x1,
      y1,
      r,
      s,
      bt1_output_script
    );
    System.out.println("java verify");
    System.out.println(res);
*/

    System.out.println("cpp verify");
    System.out.println(res);
  }
}
