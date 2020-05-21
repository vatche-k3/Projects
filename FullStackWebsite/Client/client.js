console.log('Hello World!');

const form = document.querySelector('form'); // client side javascript when there is document
const loadingElement = document.querySelector('div.loading');
const thoughtsElement = document.querySelector('div.thoughts');
const API_URL = window.location.hostname === 'localhost' ? 'http://localhost:5000/thoughts' : 'https://thoughts-api.now.sh/thoughts';

loadingElement.style.display = '';

listAllPosts();

form.addEventListener('submit', (event) => {
    event.preventDefault();
    const formData = new FormData(form);
    const name = formData.get('name');
    const content = formData.get('content');

    const post = {
        name,
        content
    };
    
    console.log(post);
    form.style.display = 'none';
    loadingElement.style.display = '';

    fetch(API_URL, {
        method: 'POST',
        body: JSON.stringify(post),
        headers:{
            'content-type': 'application/json'
        }
    }).then(response => response.json())
    .then(createdThought => {
        console.log(createdThought);
        form.reset();
        setTimeout(() => {
            form.style.display = '';
        }, 30000);
        listAllPosts();
        loadingElement.style.display = 'none';
    });
});

function listAllPosts(){
    thoughtsElement.innerHTML = '';
    fetch(API_URL)
        .then(response => response.json())
        .then(thoughts => {
            console.log(thoughts);
            thoughts.reverse();
            thoughts.forEach(post => {
                const div = document.createElement('div');

                const header = document.createElement('h3');
                header.textContent = post.name;

                const contents = document.createElement('p');
                contents.textContent = post.content;

                const date = document.createElement('small');
                date.textContent = new Date(post.created);

                div.appendChild(header);
                div.appendChild(contents);
                div.appendChild(date);

                thoughtsElement.appendChild(div);
            });
            loadingElement.style.display = 'none';
        });
}