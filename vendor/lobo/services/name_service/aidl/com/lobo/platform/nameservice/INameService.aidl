package com.lobo.platform.nameservice;

interface INameService {
    String resolveName(String key);
    boolean registerName(String key, String value);
    boolean unregisterName(String key);
}
