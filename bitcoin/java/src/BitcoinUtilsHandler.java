import org.apache.thrift.TException;

// Generated code
import bitcoin_utils.*;

import org.bitcoinj.core.*;
import org.bitcoinj.params.TestNet3Params;
import org.bitcoinj.kits.WalletAppKit;
import org.bitcoinj.utils.BriefLogFormatter;
import org.bitcoinj.wallet.WalletTransaction ;
import org.bitcoinj.crypto.TransactionSignature;
import org.bitcoinj.script.Script;
import org.bitcoinj.core.Transaction.SigHash;
import org.bitcoinj.script.ScriptChunk;

import com.google.common.util.concurrent.ListenableFuture;

import org.spongycastle.crypto.params.ECDomainParameters;
import org.spongycastle.math.ec.ECPoint;
import org.spongycastle.math.ec.ECCurve;

import java.io.File;
import java.nio.ByteBuffer;

import org.apache.log4j.*;

import java.util.List;
import java.math.BigInteger;
import java.util.concurrent.TimeUnit;


public class BitcoinUtilsHandler implements BitcoinUtils.Iface {

  private NetworkParameters params;
  public WalletAppKit kit;

  private void printl(){
    System.out.println(
      "----------------------------------------"
    );
  }

  public BitcoinUtilsHandler(String prefix) {
    System.out.println("BitcoinUtilsHandler start init");
    Logger logger = LogManager.getRootLogger();
    logger.setLevel(Level.OFF);

    params = TestNet3Params.get();
    kit = new WalletAppKit(params, new File("."), prefix);

    Wallet.SendRequest.DEFAULT_FEE_PER_KB = Coin.valueOf(6000);

    kit.startAsync();
    kit.awaitRunning();
    System.out.println("BitcoinUtilsHandler finish init");
    
    /*
    System.out.println("Balance: " + kit.wallet().getBalance());
    Iterable<WalletTransaction> transactions = kit.wallet().getWalletTransactions();
    for(WalletTransaction t: transactions) {
      System.out.println("-------------------------------------");
      System.out.println("  " + t);
      System.out.println("  " + t.getTransaction());
    }
    */
  }

  final protected static char[] hexArray = "0123456789ABCDEF".toCharArray();
  public static String bytesToHex(byte[] bytes) {
    char[] hexChars = new char[bytes.length * 2];
    for ( int j = 0; j < bytes.length; j++ ) {
      int v = bytes[j] & 0xFF;
      hexChars[j * 2] = hexArray[v >>> 4];
      hexChars[j * 2 + 1] = hexArray[v & 0x0F];
    }
    return new String(hexChars);
  }

  public void ping() {
    System.out.println("ping()");
  }

  public void printBalance(){
    System.out.println("Balance: " + kit.wallet().getBalance());
  }

  public void printTransaction(ByteBuffer btx){
    Transaction tx = new Transaction(kit.params(), btx.array());
    System.out.println(tx);
  }

  public void printAddress(ByteBuffer baddr){
    Address addr = new Address(kit.params(), baddr.array());
    System.out.println(addr);
  }

  public ByteBuffer freshAddress() {
    Address freshAddress = kit.wallet().freshReceiveKey().toAddress(params);

    System.out.println("Fresh address:");
    System.out.println(freshAddress);
    System.out.println("Hex bytes:");
    System.out.println(bytesToHex(freshAddress.getHash160()));
    return ByteBuffer.wrap(freshAddress.getHash160());
  }

  private ECKey getECKeyFromXY(ByteBuffer bx, ByteBuffer by) {
    BigInteger ix = new BigInteger(1, bx.array());
    BigInteger iy = new BigInteger(1, by.array());

    // System.out.println(ix);
    // System.out.println(iy);

    ECDomainParameters ec_params = ECKey.CURVE; 
    ECCurve ec_curve = ec_params.getCurve(); 

    ECPoint ec_point = ec_curve.createPoint(ix, iy, false);

    ECKey pk = ECKey.fromPublicOnly(ec_point);

    System.out.println("getECKeyFromXY compressed: " + pk.isCompressed());
    return pk;
  }

  private Address getAddressFromXY(ByteBuffer bx, ByteBuffer by) {
    ECKey pk = getECKeyFromXY(bx, by);
    Address address = pk.toAddress(kit.params());

    return address;
  }

  public ByteBuffer createSend(ByteBuffer bx, ByteBuffer by, int value) {
    System.out.println("createSend");
    Transaction transaction;

    Address address = getAddressFromXY(bx, by);

    System.out.println("Address:");
    System.out.println(address);
    System.out.println("Address hex bytes: " + bytesToHex(address.getHash160()));

    Coin coin = Coin.valueOf(value);

    try {
      transaction = kit.wallet().createSend(address, coin);

      if (transaction == null) {
        System.out.println("Transaction is null");
      }
      /*
      System.out.println("Transaction:");
      System.out.println(transaction);
      */
      // System.out.println("Transaction hex bytes: " + bytesToHex(transaction.bitcoinSerialize()));
    } catch (Exception e) {
      System.out.println("Error! Error! " + e.getMessage());
      System.out.println(e.toString());
      return ByteBuffer.wrap(new byte[0]);
    }
    System.out.println(transaction);
    return ByteBuffer.wrap(transaction.bitcoinSerialize());
  }

  public ByteBuffer createSendTransactionToAddress(ByteBuffer tx, ByteBuffer x, ByteBuffer y, ByteBuffer dst) {
    System.out.println("createSendTransactionToAddress");
    Transaction src_tx = new Transaction(kit.params(), tx.array());
    /*
    System.out.println("Transaction:");
    System.out.println(src_tx);
    */

    Address src_addr = getAddressFromXY(x, y);
    Address dst_addr = new Address(kit.params(), dst.array());

    List<TransactionOutput> outputs = src_tx.getOutputs();
    TransactionOutput output = null;
    for(TransactionOutput o: outputs) {
      if (o.getAddressFromP2PKHScript(kit.params()).equals(src_addr)) {
        output = o;
      }
    }
    // printl();
    if (output == null) {
      // System.out.println("Didn't find correct output");
      return ByteBuffer.wrap(new byte[0]);
    } else {
      // System.out.println("Found correct output: " + output);
    }
    // printl();

    Transaction result = new Transaction(kit.params());
    result.addInput(output);
    result.addOutput(
      output.getValue().subtract(Wallet.SendRequest.DEFAULT_FEE_PER_KB),
      dst_addr
    );
    /*
    System.out.println("T2");
    System.out.println(result);
    System.out.println("Min fee");
    System.out.println(Transaction.REFERENCE_DEFAULT_MIN_TX_FEE);
    printl();
    */
    System.out.println(result);
    return ByteBuffer.wrap(result.bitcoinSerialize());
  }

  private TransactionOutput getOutput(
    Transaction tx,
    Address dst
  ) {
    List<TransactionOutput> outputs = tx.getOutputs();
    TransactionOutput output = null;
    for(TransactionOutput o: outputs) {
      if (o.getAddressFromP2PKHScript(kit.params()).equals(dst)) {
        output = o;
      }
    }

    return output;
  }

  /* Get the output of btx = P2PKH for pk = (x, y) */
  public ByteBuffer getOutputScript(
    ByteBuffer btx, 
    ByteBuffer x, 
    ByteBuffer y
  ) {
    Transaction tx = new Transaction(kit.params(), btx.array());
    Address dst_addr = getAddressFromXY(x, y);

    TransactionOutput output = getOutput(tx, dst_addr);

    return ByteBuffer.wrap(output.getScriptBytes());
  }

  public ByteBuffer hashForSignature(
    ByteBuffer btx, 
    ByteBuffer x, 
    ByteBuffer y,
    ByteBuffer bredeem_script
  ) {
    printl();
    System.out.println("Hash for signature");
    printl();

    Transaction tx = new Transaction(kit.params(), btx.array());
    System.out.println("Transaction");
    System.out.println(tx);
    printl();

    Script redeem_script = new Script(bredeem_script.array());
    System.out.println("Redeem script");
    System.out.println(redeem_script);
    printl();

    // Address addr = getAddressFromXY(x, y);
    // TransactionOutput output = getOutput(tx, addr);

    Sha256Hash hash = tx.hashForSignature(
      // output.getIndex(),
      0,
      redeem_script,
      SigHash.ALL,
      false
    );
    return ByteBuffer.wrap(hash.getBytes());
  }

  public ByteBuffer getSignedTransaction(
    ByteBuffer btx, 
    ByteBuffer bx, 
    ByteBuffer by,
    ByteBuffer br, 
    ByteBuffer bs,
    ByteBuffer bredeem_script
  ) {
    printl();
    System.out.println("getSignedTransaction");
    printl();

    Transaction tx = new Transaction(kit.params(), btx.array());
    System.out.println("Transaction");
    System.out.println(tx);
    printl();

    Script redeem_script = new Script(bredeem_script.array());
    System.out.println("Redeem script");
    System.out.println(redeem_script);
    printl();

    BigInteger x = new BigInteger(1, bx.array());
    BigInteger y = new BigInteger(1, by.array());
    BigInteger r = new BigInteger(1, br.array());
    BigInteger s = new BigInteger(1, bs.array());

    System.out.println("x: " + x);
    System.out.println("y: " + y);
    System.out.println("r: " + r);
    System.out.println("s: " + s);
    printl();

    ECKey pk = getECKeyFromXY(bx, by);

    TransactionSignature sig = new TransactionSignature(
      r,
      s
    );
    Script input_script = new Script(
      Script.createInputScript(
        sig.encodeToBitcoin(),
        pk.getPubKey()
      )
    );
    System.out.println("input_script");
    System.out.println(input_script);
    System.out.println("/input_script");

    printl();
    try {
      input_script.correctlySpends(
          tx,
          0,
          redeem_script
          );
    } catch (Exception e) {
      
      System.out.println(e.toString());
      return ByteBuffer.wrap(new byte[0]);
    }
    
    tx.getInput(0).setScriptSig(input_script);
    System.out.println("Hip hip! Hurray! (verified signaure)");
    printl();
    return ByteBuffer.wrap(tx.bitcoinSerialize());
  }

  public boolean cppVerifySignature(ByteBuffer data, ByteBuffer x, ByteBuffer y, ByteBuffer r, ByteBuffer s) {
    System.out.println("Error Error cpp called in java server");
    return false;
  }

  public boolean verifyTransaction(ByteBuffer btx, ByteBuffer x, ByteBuffer y, int value) {
    System.out.println("verifyTransaction");

    Transaction tx = new Transaction(kit.params(), btx.array());
    TransactionOutput output = getOutput(tx, getAddressFromXY(x, y));
    if (output == null) {
      System.out.println("verifyTransaction no matching output");
      return false;
    }

    if (output.getValue().getValue() != value) {
      System.out.println("verifyTransaction incorrect value");
      System.out.println(output.getValue().getValue());
      System.out.println(value);
      return false;
    }
 
    System.out.println("verifyTransaction finished");
    return true;
  }

  public boolean broadcastTransaction(ByteBuffer btx) {
    System.out.println("broadcastTransaction");
    Transaction tx = new Transaction(kit.params(), btx.array());
    System.out.println(tx);
    try {
      // kit.peerGroup().broadcastTransaction(tx).future().get(20, TimeUnit.SECONDS);
      kit.peerGroup().broadcastTransaction(tx).future().get();
    } catch (Exception e) {
      System.out.println("broadcastTransaction failed");
      System.out.println(e);
      return false;
    }
    System.out.println("broadcastTransaction finished");
    return true;
  }

  private void waitMinute(TransactionConfidence confidence, int depth) {
    try {
      System.out.println(confidence);
      confidence.getDepthFuture(depth).get(1, TimeUnit.MINUTES);
    } catch (Exception e) {
    }
  }

  public boolean waitForTransactionHelper(Transaction tx, int depth) {
    System.out.println(tx);

    try {
      for(int i = 0; i < 60; ++i) {
        System.out.println(i);
        TransactionConfidence confidence = tx.getConfidence();
        waitMinute(confidence, depth);
      }
      TransactionConfidence confidence = tx.getConfidence();
      System.out.println(confidence);
      confidence.getDepthFuture(depth).get(1, TimeUnit.MINUTES);
    } catch (Exception e) {
      System.out.println("waitForTransaction failed");
      System.out.println(e);
      return false;
    }
    System.out.println("waitForTransaction finished");
    return true;
  }
 
  public boolean waitForTransaction(ByteBuffer btx, int depth) {
    System.out.println("waitForTransaction");
    Transaction tx = new Transaction(kit.params(), btx.array());

    for(TransactionOutput o: tx.getOutputs()) {
      kit.wallet().addWatchedAddress(o.getAddressFromP2PKHScript(kit.params()));
      System.out.println("Adding watched address");
      System.out.println(o.getAddressFromP2PKHScript(kit.params()));
    }

    return waitForTransactionHelper(tx, depth);
  }

  private Transaction findTransactionByOutput(Address addr) {

    Transaction tx = null;
    Iterable<WalletTransaction> txs = kit.wallet().getWalletTransactions();
    for(WalletTransaction wtx: txs) {
      TransactionOutput o = getOutput(wtx.getTransaction(), addr);
      if (o != null) {
        tx = wtx.getTransaction();
      }
    }
    return tx;
  }

  public boolean waitForTransactionByOutput(ByteBuffer baddr, int depth) {
    System.out.println("waitForTransactionByOutput");
    Address addr = new Address(kit.params(), baddr.array());
    kit.wallet().addWatchedAddress(addr);
    System.out.println("Adding watched address");

    Transaction tx = null;
    
    try{
      while(tx == null){
        Thread.sleep(10000);
        System.out.println("Searching for transaction");
        tx = findTransactionByOutput(addr); 
     }
      
      System.out.println("Found transaction");
      System.out.println(tx);

      return waitForTransactionHelper(tx, depth);
    }
    catch (Exception e) {
      System.out.println(e);
      return false;
    }
  }

  public IntegerPair getSignature(ByteBuffer baddr){
    System.out.println("get signature");
    Address addr = new Address(kit.params(), baddr.array());
    System.out.println(addr);
    Transaction tx = findTransactionByOutput(addr);
    printl();
    System.out.println(tx);
    printl();
    TransactionInput input = tx.getInput(0);
    Script input_script = input.getScriptSig();
    List<ScriptChunk> chunks = input_script.getChunks();
    /*
    System.out.println("chunks");
    for(ScriptChunk chunk: chunks) {
      System.out.println(chunk);
    }
    */
    ECKey.ECDSASignature signature = ECKey.ECDSASignature.decodeFromDER(chunks.get(0).data);
    System.out.println("-- r s");
    System.out.println(signature.r);
    System.out.println(signature.s);
    return new IntegerPair(
      ByteBuffer.wrap(signature.r.toByteArray()),
      ByteBuffer.wrap(signature.s.toByteArray())
    );
    // return ByteBuffer.wrap(chunks.get(0).data);
  }
}
