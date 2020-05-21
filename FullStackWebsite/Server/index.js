const express = require('express');
const cors = require('cors');
const monk = require('monk');
const Filter = require('bad-words');
const rateLimit = require("express-rate-limit");

const app = express();

const db = monk(process.env.MONGO_URI || 'localhost/FullStackWebsite');
const thoughts = db.get('thoughts');
const filter = new Filter();

app.use(cors());
app.use(express.json());


app.get('/', (req, res)=> {
    res.json({
        message: 'A place to post your thoughts'
    });
});

app.get('/thoughts', (req, res) => {
    thoughts
        .find()
        .then(thoughts => {
            res.json(thoughts);
        });
})

function isValidThought(post){
    return post.name && post.name.toString().trim() !== '' &&
    post.content && post.content.toString().trim() !== '';
}

app.use(rateLimit({
    windowMs: 30 * 1000,
    max: 1
}));

app.post('/thoughts', (req, res) =>{
    if(isValidThought(req.body)){
        const post = {
            name: filter.clean(req.body.name.toString()),
            content: filter.clean(req.body.content.toString()),
            created: new Date()
        };
        console.log(post);

        thoughts
            .insert(post)
            .then(createdThought => {
                res.json(createdThought);
            });
    }
    else{
        res.status(422);
        res.json({
            message: 'Name and content required bro'
        });
    }
    console.log(req.body);
});

app.listen(5000, () =>{
    console.log('Listening on http://localhost:5000');
});