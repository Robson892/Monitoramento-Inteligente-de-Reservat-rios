const express = require('express');
const app = express();
const port = 3000;
const bodyParser = require('body-parser');

let levels = {
    1: 0,
    2: 0,
    3: 0,
    4: 0,
    5: 0,
    6: 0
}; // Armazena o nível da água para 6 casas

// Para ler dados JSON
app.use(bodyParser.json());

// Endpoint para receber dados do sensor
app.post('/update/:house', (req, res) => {
    const houseId = req.params.house;
    const level = req.body.level;
    
    if (levels[houseId] !== undefined) {
        levels[houseId] = level; // Atualiza o nível da água para a casa específica
        console.log(`Dados recebidos para casa ${houseId}:`, level);
        res.sendStatus(200);
    } else {
        res.status(400).json({ error: 'Casa não encontrada' });
    }
});

// Endpoint para enviar os níveis da água para o HTML
app.get('/levels', (req, res) => {
    res.json(levels); // Envia os níveis da água para todas as casas
});

// Serve arquivos estáticos da pasta 'public'
app.use(express.static('public'));

// Inicie o servidor
app.listen(port, () => {
    console.log(`Servidor rodando em http://192.168.68.68:${port}`);
});
