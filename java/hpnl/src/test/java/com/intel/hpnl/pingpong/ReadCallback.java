package com.intel.hpnl.pingpong;

import java.util.Arrays;
import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EqService;

public class ReadCallback implements Handler {
  public ReadCallback(boolean is_server, EqService eqService) {
    this.is_server = is_server;
    this.eqService = eqService;
    byteArray = new byte[4096];
    Arrays.fill(byteArray, (byte)'0');
    buffer = ByteBuffer.wrap(byteArray);
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize, int blockBufferId, long seq) {
    if (!is_server) {
      if (count == 0) {
        startTime = System.currentTimeMillis();
      }
      if (++count >= 1000000) {
        endTime = System.currentTimeMillis();
        totally_time = (float)(endTime-startTime)/1000;
        System.out.println("finished, total time is " + totally_time + " s");
        eqService.shutdown();
        return;
      }
    }
    con.send(buffer, 4096, 0, 0, seq);
  }
  private int count = 0;
  private long startTime;
  private long endTime;
  private float totally_time = 0;

  private ByteBuffer buffer;
  private byte[] byteArray;
  boolean is_server = false;
  EqService eqService;
}