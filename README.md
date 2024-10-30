# eMoney

A modern cryptocurrency implementing BSV op-codes with dynamic block sizing.

## Features

- SHA512-based Proof of Work
- BSV-compatible op-code set
- Dynamic block size (doubles with each halving)
- Initial 1MB block size
- Integrated P2P poker
- Maximum supply of 50 million coins
- 50 coins per block initial reward

## Building

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake libssl-dev boost-dev

# macOS
brew install cmake openssl boost

# Windows
# Install Visual Studio 2019 or later with C++ support
# Install vcpkg and required packages
```

### Compilation

```bash
mkdir build
cd build
cmake ..
make
```

## Development

### Directory Structure

```
eMoney/
├── src/
│   ├── core/       # Core blockchain implementation
│   ├── crypto/     # Cryptographic primitives
│   ├── network/    # P2P networking
│   ├── wallet/     # Wallet implementation
│   ├── mining/     # Mining code
│   └── script/     # Script engine
├── docs/           # Documentation
├── tests/          # Test suite
└── contrib/        # Additional tools
```

### Testing

```bash
cd build
make test
```

## Mining

To start mining:
```bash
./emoney-miner --address=your_address
```

## Poker

To start a poker game:
```bash
./emoney-poker --table=new
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

* Bitcoin SV for op-code inspiration
* Satoshi Nakamoto for the original blockchain design
