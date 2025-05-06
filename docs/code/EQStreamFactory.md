# File: `EQStreamFactory.h`

## Classes

- `EQStreamFactory`

## Functions

- `void CheckTimeout(bool remove_all = false);`
- `void Push(EQStream *s);`
- `bool loadPublicKey();`
- `bool Open();`
- `bool Open(unsigned long port) { Port=port; return Open(); }`
- `void Close();`
- `void ReaderLoop();`
- `void WriterLoop();`
- `void CombinePacketLoop();`
- `void Stop() { StopReader(); StopWriter(); StopCombinePacket(); }`
- `void StopReader() { MReaderRunning.lock(); ReaderRunning=false; MReaderRunning.unlock(); }`
- `void StopWriter() { MWriterRunning.lock(); WriterRunning=false; MWriterRunning.unlock(); WriterWork.Signal(); }`
- `void StopCombinePacket() { MCombinePacketRunning.lock(); CombinePacketRunning=false; MCombinePacketRunning.unlock(); }`
- `void SignalWriter() { WriterWork.Signal(); }`

## Notable Comments

- /*
- */
