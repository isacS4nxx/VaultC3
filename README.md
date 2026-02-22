# VaultC3
Vault for password with esp32 C3 0.42 Oled

<img width="661" height="604" alt="Screenshot from 2026-02-22 16-13-02" src="https://github.com/user-attachments/assets/d6d1aa43-bd57-4808-85ea-7e48db5e5b33" />
![83106](https://github.com/user-attachments/assets/c896ef96-cf9d-4735-98fa-f935ec3042b7)

Entendido. Aqui está a versão da documentação para o seu GitHub com um tom profissional, direto e humano, sem o uso de emojis:

Vault-C3: Gerenciador de Senhas Portátil
O Vault-C3 é um dispositivo físico e independente para armazenamento de credenciais, construído com um microcontrolador ESP32-C3 e um display OLED de 0.42 polegadas.

Este projeto nasceu da necessidade de um gerenciador de senhas que não dependesse de serviços de terceiros ou conexões com a nuvem. O dispositivo cria seu próprio ponto de acesso Wi-Fi seguro, permitindo o gerenciamento de dados através de uma interface web minimalista e protegida.

Funcionalidades Principais
Rede Isolada: O dispositivo opera como um Access Point (AP), o que significa que seus dados nunca trafegam pela internet.

Interface Web Moderna: Painel administrativo com design focado em usabilidade para cadastro de nomes, URLs e senhas.

Gerador de Senhas: Função integrada que utiliza a API de criptografia do navegador para criar chaves aleatórias de 16 caracteres.

Monitoramento Visual: Display OLED que exibe um carrossel dos serviços salvos a cada 5 segundos.

Privacidade por Design: Senhas são ocultadas na interface web através de máscaras visuais, sendo reveladas apenas sob comando.

Especificações de Hardware
Placa Principal: ESP32-C3 (Super Mini ou equivalente).

Display: OLED 0.42 polegadas (Protocolo I2C).

Conexões:

SDA: GPIO 5

SCL: GPIO 6

Alimentação: 3.3V e GND

Guia de Instalação e Configuração
1. Requisitos de Software
Certifique-se de ter a Arduino IDE configurada para a arquitetura ESP32 e instale as seguintes dependências:

Adafruit SSD1306

Adafruit GFX Library

ESPAsyncWebServer

AsyncTCP

2. Calibração do Display
Devido às características físicas da tela de 0.42 polegadas, o código utiliza um deslocamento específico para garantir a legibilidade:

Offset Horizontal (X): 28

Offset Vertical (Y): 24

3. Upload do Firmware
Conecte o dispositivo via USB.

Selecione o modelo de placa ESP32C3 Dev Module.

Carregue o código fornecido no arquivo principal deste repositório.

Instruções de Uso
Energize o dispositivo em uma porta USB ou bateria externa.

Conecte-se à rede Wi-Fi denominada Vault-C3-Safe utilizando a senha padrão 12345678.

No seu navegador, acesse o endereço IP 192.168.4.1.

Realize a autenticação com a Senha Master: 123.

Após o login, você poderá visualizar, adicionar ou remover credenciais.

Considerações de Segurança
Este projeto foi desenvolvido aplicando princípios de cibersegurança defensiva:

Prevenção de Injeção: Tratamento e sanitização de todas as strings recebidas via requisições HTTP.

Segurança de Memória: Uso da biblioteca Preferences para armazenamento persistente na Flash, evitando a exposição de arquivos em sistemas de arquivos abertos.

Entropia: O gerador de senhas utiliza métodos criptográficos nativos do sistema operacional do cliente para garantir aleatoriedade real.
Sanitização de Entradas: Proteção contra injeção de scripts no painel.

Criptografia no Browser: O gerador de senhas usa window.crypto para garantir entropia real.

Mascaramento Visual: Senhas borradas por padrão para evitar "shoulder surfing".

Feito com ☕ e persistência por [S4nxx + Gemini].
